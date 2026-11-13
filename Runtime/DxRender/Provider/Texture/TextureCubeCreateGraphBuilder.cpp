module DxRender.Provider;

import :TextureCubeCreateGraphBuilder;
import :TextureUtils;
import DxRender.Constants;
import Core.Utils;
import Core.Bit;
import Core.Logger;

TextureCubeCreateGraphBuilder::TextureCubeCreateGraphBuilder(
    TaskScheduler& taskScheduler,
    ResourceFactory& resFactory,
    DescriptorFactory& descFactory)
    : m_taskScheduler{ taskScheduler }
    , m_resFactory{ resFactory }
    , m_descFactory{ descFactory }
{}

void TextureCubeCreateGraphBuilder::LoadTextureCubes(const std::vector<TextureCubeLoadRequest>& requests)
{
    m_idGenerator.Reset();
    m_graph.Reset();

    std::size_t totalUploadSize = 0;
    auto textureUploads = std::make_shared<std::vector<TextureCubeUploadEntry>>(
        BuildTextureCubeUploads(requests, totalUploadSize));
    if (textureUploads->empty())
        return; // 만들지 못한 큐브는 준비되지 않은 채로 남는다 (원인은 만든 곳에서 남김)

    Resource uploadBuffer = m_resFactory.CreateResource(totalUploadSize, ResInitType::Upload);
    if (!uploadBuffer)
        return; // 원인은 Device가 남김. 이번 묶음의 큐브는 준비되지 않는다

    RGResourceID uploadResID = m_idGenerator.Generate();

    // 큐브 텍스처도 persistent 리소스이므로, 이번 그래프 구간만 상태를 빌려온다(Import)가 끝나면 SRV로 반환(Export)한다.
    for (const auto& upload : *textureUploads)
        m_graph.ImportResource(upload.resID, RGAccess::CopyDest);

    BuildUploadPass(textureUploads, uploadResID);

    for (const auto& upload : *textureUploads)
        m_graph.ExportResource(upload.resID, RGAccess::SRV);

    auto compiledTasks = m_graph.Compile();

    auto resContext = CreateResourceContext(textureUploads, uploadResID, std::move(uploadBuffer));
    m_taskScheduler.SubmitTask(compiledTasks, resContext);
    m_graph.Reset(); // 패스의 실행 함수가 업로드 목록(에셋과 리소스)을 들고 있으므로 다음 묶음까지 붙잡지 않게 비운다
}

std::vector<TextureCubeUploadEntry> TextureCubeCreateGraphBuilder::BuildTextureCubeUploads(
    const std::vector<TextureCubeLoadRequest>& requests,
    std::size_t& outTotalUploadSize)
{
    std::vector<TextureCubeUploadEntry> uploads;
    uploads.reserve(requests.size());

    std::size_t offset = 0;
    for (const auto& req : requests)
    {
        auto resDesc = CreateTextureCubeDesc(*req.asset);
        auto texRes = m_resFactory.CreateTextureResource(resDesc);
        if (!texRes)
            continue; // 원인은 Device가 남김

        auto& textureCubeResource = req.resource;
        textureCubeResource->Set(texRes);
        textureCubeResource->SetSize(Core::Size{ req.asset->width, req.asset->height });
        if (!CreateTextureCubeViews(m_descFactory, textureCubeResource.get()))
        {
            Core::Logger::Error("TextureCubeCreateGraphBuilder: 큐브 텍스처의 뷰를 만들지 못해 건너뛴다 ({}x{})",
                req.asset->width, req.asset->height);
            textureCubeResource->Set(Resource{});
            continue;
        }

        RGResourceID texResID = m_idGenerator.Generate();

        offset = Core::AlignUp(offset, TextureAlignment::Placement);
        uploads.push_back({ texResID, textureCubeResource, req.asset, offset });

        // 6면 * mipCount 개의 subresource 크기 합산
        offset += m_resFactory.GetRequiredIntermediateSize(
            resDesc, 0, req.asset->mipCount * req.asset->faceCount, offset);
    }

    outTotalUploadSize = Core::AlignUp(offset, TextureAlignment::Placement);
    return uploads;
}

std::shared_ptr<ResourceContext> TextureCubeCreateGraphBuilder::CreateResourceContext(
    std::shared_ptr<std::vector<TextureCubeUploadEntry>> textureUploads,
    RGResourceID uploadResID,
    Resource uploadBuffer)
{
    auto* rawContext = new ResourceContext(m_idGenerator.Count());

    for (const auto& upload : *textureUploads)
        rawContext->Set(upload.resID, upload.resource->Get());
    rawContext->Set(uploadResID, std::move(uploadBuffer));

    // GPU 작업 완료 후 TaskScheduler가 컨텍스트를 놓는 시점에 finalize.
    return std::shared_ptr<ResourceContext>(
        rawContext,
        [this, textureUploads](ResourceContext* ctx) mutable
        {
            FinalizeTextureCubes(*textureUploads);
            delete ctx;
        });
}

void TextureCubeCreateGraphBuilder::BuildUploadPass(
    std::shared_ptr<std::vector<TextureCubeUploadEntry>> textureUploads,
    RGResourceID uploadResID)
{
    auto& upload = m_graph.AddCopyPass("TextureCubeUpload");

    for (auto& tex : *textureUploads)
        upload.Write(tex.resID, RGAccess::CopyDest);

    upload.execute =
        [
            this,
            textureUploads,
            uploadResID
        ]
        (TaskCommandLists cmds, TaskContext& ctx) mutable
        {
            CommandList& cmd = cmds.Single();

            auto& uploadRes = ctx.GetResource(uploadResID);
            for (auto& tex : *textureUploads)
                UploadTextureCube(cmd, *tex.asset, tex.resource->Get(), uploadRes, tex.offset); // subImages[mip + face * mipCount] 순서로 6면*N밉 전부 CopyTextureRegion
        };
}

void TextureCubeCreateGraphBuilder::FinalizeTextureCubes(std::vector<TextureCubeUploadEntry>& textureUploads)
{
    for (auto& tex : textureUploads)
        tex.resource->MarkReady();
}