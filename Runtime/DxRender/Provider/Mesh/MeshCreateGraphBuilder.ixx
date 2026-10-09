export module DxRender.Provider:MeshCreateGraphBuilder;

import std;
import :MeshLoadRequest;
import :MeshUtils;
import DxRender.Resource;
import DxRender.Constants;
import DxRender.RGResourceID;
import DxRender.Graph;
import DxRender.Definition;
import DxRender.Task;
import DxRender.Factory;
import Core.Bit;
import Contract.Asset.Data;

struct MeshUploadEntry
{
    RGResourceID vbResID{ InvalidRGID };
    RGResourceID ibResID{ InvalidRGID };

    UploadRegion vbRegion;
    UploadRegion ibRegion;

    std::shared_ptr<MeshResource> resource;
    std::shared_ptr<MeshAsset> asset;

    std::uint32_t vbHeapIndex{ std::numeric_limits<std::uint32_t>::max() };
    std::uint32_t ibHeapIndex{ std::numeric_limits<std::uint32_t>::max() };
};

export class MeshCreateGraphBuilder
{
public:
    ~MeshCreateGraphBuilder() = default;
    MeshCreateGraphBuilder() = delete;

    MeshCreateGraphBuilder(
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory) :
        m_taskScheduler{ taskScheduler },
        m_resFactory{ resFactory },
        m_descFactory{ descFactory }
    {}

    void LoadMeshes(const std::vector<MeshLoadRequest>& requests)
    {
        m_idGenerator.Reset();
        m_graph.Reset();

        std::size_t totalUploadSize = 0;
        auto meshUploads = std::make_shared<std::vector<MeshUploadEntry>>(
            BuildMeshUploads(requests, totalUploadSize));

        RGResourceID uploadResID = m_idGenerator.Generate();

        for (const auto& mesh : *meshUploads)
        {
            m_graph.ImportResource(mesh.vbResID, RGAccess::CopyDest);
            m_graph.ImportResource(mesh.ibResID, RGAccess::CopyDest);
        }

        BuildUploadPass(meshUploads, uploadResID);

        for (const auto& mesh : *meshUploads)
        {
            m_graph.ExportResource(mesh.vbResID, RGAccess::SRV);
            m_graph.ExportResource(mesh.ibResID, RGAccess::SRV);
        }

        auto compiledTasks = m_graph.Compile();

        auto resContext = CreateResourceContext(meshUploads, uploadResID, totalUploadSize);
        m_taskScheduler.SubmitTask(compiledTasks, resContext);
    }

private:
    std::vector<MeshUploadEntry> BuildMeshUploads(
        const std::vector<MeshLoadRequest>& requests,
        std::size_t& outTotalUploadSize)
    {
        std::vector<MeshUploadEntry> uploads;
        uploads.reserve(requests.size());

        std::size_t offset = 0;
        for (const auto& req : requests)
        {
            RGResourceID vbResID = m_idGenerator.Generate();
            RGResourceID ibResID = m_idGenerator.Generate();

            auto vbRes = m_resFactory.CreateResource(static_cast<std::uint64_t>(req.vbBytes), ResInitType::Default);
            auto ibRes = m_resFactory.CreateResource(static_cast<std::uint64_t>(req.ibBytes), ResInitType::Default);

            // SRV는 실제 데이터 업로드 여부와 무관하게 만들 수 있으므로, 리소스 생성 직후 바로 구해둔다.
            auto indexCount = static_cast<std::uint32_t>(req.asset->indices.size());
            std::uint32_t vbHeapIndex = m_descFactory.CreateBufferSRV(
                DescriptorAllocationType::Persistent, vbRes, 0, req.asset->vertexCount, req.asset->vertexStride);
            std::uint32_t ibHeapIndex = m_descFactory.CreateBufferSRV(
                DescriptorAllocationType::Persistent, ibRes, 0, indexCount, sizeof(std::uint32_t));

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

    std::shared_ptr<ResourceContext> CreateResourceContext(
        std::shared_ptr<std::vector<MeshUploadEntry>> meshUploads,
        RGResourceID uploadResID,
        std::size_t totalUploadSize)
    {
        auto* rawContext = new ResourceContext(m_idGenerator.Count());

        for (const auto& mesh : *meshUploads)
        {
            rawContext->Set(mesh.vbResID, mesh.vbRegion.dstBuffer);
            rawContext->Set(mesh.ibResID, mesh.ibRegion.dstBuffer);
        }
        rawContext->Set(uploadResID, m_resFactory.CreateResource(totalUploadSize, ResInitType::Upload));

        return std::shared_ptr<ResourceContext>(
            rawContext,
            [this, meshUploads](ResourceContext* ctx) mutable
            {
                FinalizeMeshes(*meshUploads);
                delete ctx;
            });
    }

    void BuildUploadPass(
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
                    UploadBufferRegion(cmd, uploadRes, mesh.vbRegion);
                    UploadBufferRegion(cmd, uploadRes, mesh.ibRegion);
                }
            };
    }

    void FinalizeMeshes(std::vector<MeshUploadEntry>& meshUploads)
    {
        for (auto& mesh : meshUploads)
        {
            auto* meshRes = static_cast<StaticMeshResource*>(mesh.resource.get());

            auto vertexCount = static_cast<std::uint32_t>(mesh.asset->vertices.size());
            auto indexCount = static_cast<std::uint32_t>(mesh.asset->indices.size());

            meshRes->SetResource(std::move(mesh.vbRegion.dstBuffer), std::move(mesh.ibRegion.dstBuffer), vertexCount, indexCount);
            meshRes->SetVertexHeapIndex(mesh.vbHeapIndex);
            meshRes->SetIndexHeapIndex(mesh.ibHeapIndex);
            if (mesh.asset->format == VertexFormat::UI)
                meshRes->SetCPUTemplate(mesh.asset); // UI 배칭할때 CPU에서 world를 곱해 재조립할 수 있도록(gpu에서 world를 곱하면 draw call을 나눠야 함) 원본 유지

            meshRes->MarkReady();
        }
    }

    TaskScheduler& m_taskScheduler;
    ResourceFactory& m_resFactory;
    DescriptorFactory& m_descFactory;

    RenderGraph m_graph;
    RGResourceIDGenerator m_idGenerator;
};