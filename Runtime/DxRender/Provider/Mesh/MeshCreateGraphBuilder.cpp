module DxRender.Provider;

import :MeshCreateGraphBuilder;
import DxRender.Definition;
import DxRender.Resource;
import Core.Bit;
import Core.Logger;

MeshCreateGraphBuilder::MeshCreateGraphBuilder(
    TaskScheduler& taskScheduler,
    ResourceFactory& resFactory,
    DescriptorFactory& descFactory) :
    m_taskScheduler{ taskScheduler },
    m_resFactory{ resFactory },
    m_descFactory{ descFactory }
{}

void MeshCreateGraphBuilder::LoadMeshes(const std::vector<MeshLoadRequest>& requests)
{
    m_idGenerator.Reset();
    m_graph.Reset();

    std::size_t totalUploadSize = 0;
    auto meshUploads = std::make_shared<std::vector<MeshUploadEntry>>(
        BuildMeshUploads(requests, totalUploadSize));
    if (meshUploads->empty())
        return; // 만들지 못한 메시는 준비되지 않은 채로 남는다 (원인은 만든 곳에서 남김)

    Resource uploadBuffer = m_resFactory.CreateResource(totalUploadSize, ResInitType::Upload);
    if (!uploadBuffer)
        return; // 원인은 Device가 남김. 이번 묶음의 메시는 준비되지 않는다

    RGResourceID uploadResID = m_idGenerator.Generate();

    for (const auto& mesh : *meshUploads)
    {
        m_graph.ImportResource(mesh.vbResID, RGAccess::CopyDest);
        m_graph.ImportResource(mesh.ibResID, RGAccess::CopyDest);
    }

    BuildUploadPass(meshUploads, uploadResID);

    for (const auto& mesh : *meshUploads)
    {
        // 정점 셰이더가 바인드리스로 읽으므로 픽셀 전용 상태가 아니다
        m_graph.ExportResource(mesh.vbResID, RGAccess::SRVNonPixel);
        m_graph.ExportResource(mesh.ibResID, RGAccess::SRVNonPixel);
    }

    auto compiledTasks = m_graph.Compile();

    auto resContext = CreateResourceContext(meshUploads, uploadResID, std::move(uploadBuffer));
    m_taskScheduler.SubmitTask(compiledTasks, resContext);
    m_graph.Reset(); // 패스의 실행 함수가 업로드 목록(에셋과 리소스)을 들고 있으므로 다음 묶음까지 붙잡지 않게 비운다
}

std::vector<MeshUploadEntry> MeshCreateGraphBuilder::BuildMeshUploads(
    const std::vector<MeshLoadRequest>& requests,
    std::size_t& outTotalUploadSize)
{
    std::vector<MeshUploadEntry> uploads;
    uploads.reserve(requests.size());

    std::size_t offset = 0;
    for (const auto& req : requests)
    {
        auto vbRes = m_resFactory.CreateResource(static_cast<std::uint64_t>(req.vbBytes), ResInitType::Default);
        auto ibRes = m_resFactory.CreateResource(static_cast<std::uint64_t>(req.ibBytes), ResInitType::Default);
        if (!vbRes || !ibRes)
            continue; // 원인은 Device가 남김

        // SRV는 실제 데이터 업로드 여부와 무관하게 만들 수 있으므로, 리소스 생성 직후 바로 구해둔다.
        auto indexCount = static_cast<std::uint32_t>(req.asset->indices.size());
        std::uint32_t vbHeapIndex = m_descFactory.CreateBufferSRV(
            DescriptorAllocationType::Persistent, vbRes, 0, req.asset->vertexCount, req.asset->vertexStride);
        std::uint32_t ibHeapIndex = m_descFactory.CreateBufferSRV(
            DescriptorAllocationType::Persistent, ibRes, 0, indexCount, sizeof(std::uint32_t));
        if (vbHeapIndex == std::numeric_limits<std::uint32_t>::max() || ibHeapIndex == std::numeric_limits<std::uint32_t>::max())
        {
            Core::Logger::Error("MeshCreateGraphBuilder: 메시의 버퍼 뷰를 만들지 못해 건너뛴다 (정점 {}, 인덱스 {})",
                req.asset->vertexCount, indexCount);
            continue;
        }

        RGResourceID vbResID = m_idGenerator.Generate();
        RGResourceID ibResID = m_idGenerator.Generate();

        std::size_t vbOffset = offset;
        std::size_t ibOffset = offset + req.vbBytes;

        MeshUploadEntry entry;
        entry.vbResID = vbResID;
        entry.ibResID = ibResID;
        entry.vbRegion = { req.asset->vertices.data(), req.asset->vertices.size(), static_cast<std::uint64_t>(vbOffset), vbRes };
        entry.ibRegion = { req.asset->indices.data(), req.asset->indices.size() * sizeof(std::uint32_t), static_cast<std::uint64_t>(ibOffset), ibRes };
        entry.resource = req.resource;
        entry.asset = req.asset;
        entry.vbHeapIndex = vbHeapIndex;
        entry.ibHeapIndex = ibHeapIndex;

        uploads.push_back(std::move(entry));

        offset += req.vbBytes + req.ibBytes;
    }

    outTotalUploadSize = Core::AlignUp(offset, BufferAlignment::VertexBuffer);
    return uploads;
}

std::shared_ptr<ResourceContext> MeshCreateGraphBuilder::CreateResourceContext(
    std::shared_ptr<std::vector<MeshUploadEntry>> meshUploads,
    RGResourceID uploadResID,
    Resource uploadBuffer)
{
    auto* rawContext = new ResourceContext(m_idGenerator.Count());

    for (const auto& mesh : *meshUploads)
    {
        rawContext->Set(mesh.vbResID, mesh.vbRegion.dstBuffer);
        rawContext->Set(mesh.ibResID, mesh.ibRegion.dstBuffer);
    }
    rawContext->Set(uploadResID, std::move(uploadBuffer));

    return std::shared_ptr<ResourceContext>(
        rawContext,
        [this, meshUploads](ResourceContext* ctx) mutable
        {
            FinalizeMeshes(*meshUploads);
            delete ctx;
        });
}

void MeshCreateGraphBuilder::BuildUploadPass(
    std::shared_ptr<std::vector<MeshUploadEntry>> meshUploads,
    RGResourceID uploadResID)
{
    auto& upload = m_graph.AddCopyPass("MeshUpload");

    for (auto& mesh : *meshUploads)
    {
        upload.Write(mesh.vbResID, RGAccess::CopyDest);
        upload.Write(mesh.ibResID, RGAccess::CopyDest);
    }

    upload.execute =
        [
            this,
            meshUploads,
            uploadResID
        ]
        (TaskCommandLists cmds, TaskContext& ctx) mutable
        {
            CommandList& cmd = cmds.Single();

            auto& uploadRes = ctx.GetResource(uploadResID);
            for (auto& mesh : *meshUploads)
            {
                if (!UploadBufferRegion(cmd, uploadRes, mesh.vbRegion) ||
                    !UploadBufferRegion(cmd, uploadRes, mesh.ibRegion))
                    return; // 원인은 UploadBufferRegion이 남김
            }
        };
}

void MeshCreateGraphBuilder::FinalizeMeshes(std::vector<MeshUploadEntry>& meshUploads)
{
    for (auto& mesh : meshUploads)
    {
        auto* meshRes = mesh.resource.get();

        auto vertexCount = mesh.asset->vertexCount;
        auto indexCount = static_cast<std::uint32_t>(mesh.asset->indices.size());

        meshRes->SetResource(std::move(mesh.vbRegion.dstBuffer), std::move(mesh.ibRegion.dstBuffer), vertexCount, indexCount);
        meshRes->SetVertexHeapIndex(mesh.vbHeapIndex);
        meshRes->SetIndexHeapIndex(mesh.ibHeapIndex);
        if (mesh.asset->format == VertexFormat::UI)
            meshRes->SetCPUTemplate(mesh.asset); // UI 배칭할때 CPU에서 world를 곱해 재조립할 수 있도록(gpu에서 world를 곱하면 draw call을 나눠야 함) 원본 유지

        meshRes->MarkReady();
    }
}