export module Runtime.Render.Provider:TextureCubeCreateGraphBuilder;

import std;
import :TextureCubeLoadRequest;
import :TextureUtils;
import Core.Utils;
import Core.Bit;
import Runtime.Render.Resource;
import Runtime.Render.Constants;
import Runtime.Render.RGResourceID;
import Runtime.Render.Graph;
import Runtime.Render.Task;
import Runtime.Render.Factory;
import Client.Asset.Data;

struct TextureCubeUploadEntry
{
    RGResourceID resID;
    std::shared_ptr<TextureCubeResource> resource;
    std::shared_ptr<TextureCubeAsset> asset;
    std::size_t offset{ 0 };
};

export class TextureCubeCreateGraphBuilder
{
public:
    ~TextureCubeCreateGraphBuilder() = default;
    TextureCubeCreateGraphBuilder() = delete;

    TextureCubeCreateGraphBuilder(
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory)
        : m_taskScheduler{ taskScheduler }
        , m_resFactory{ resFactory }
        , m_descFactory{ descFactory }
    {
    }

    void LoadTextureCubes(const std::vector<TextureCubeLoadRequest>& requests)
    {
        m_idGenerator.Reset();
        m_graph.Reset();

        std::size_t totalUploadSize = 0;
        auto textureUploads = std::make_shared<std::vector<TextureCubeUploadEntry>>(
            BuildTextureCubeUploads(requests, totalUploadSize));

        RGResourceID uploadResID = m_idGenerator.Generate();

        // 큐브 텍스처도 persistent 리소스이므로, 이번 그래프 구간만 상태를 빌려온다(Import)가 끝나면 SRV로 반환(Export)한다.
        for (const auto& upload : *textureUploads)
            m_graph.ImportResource(upload.resID, RGAccess::CopyDest);

        BuildUploadPass(textureUploads, uploadResID);

        for (const auto& upload : *textureUploads)
            m_graph.ExportResource(upload.resID, RGAccess::SRV);

        auto compiledTasks = m_graph.Compile();

        auto resContext = CreateResourceContext(textureUploads, uploadResID, totalUploadSize);
        m_taskScheduler.SubmitTask(compiledTasks, resContext);
    }

private:
    std::vector<TextureCubeUploadEntry> BuildTextureCubeUploads(
        const std::vector<TextureCubeLoadRequest>& requests,
        std::size_t& outTotalUploadSize)
    {
        std::vector<TextureCubeUploadEntry> uploads;
        uploads.reserve(requests.size());

        std::size_t offset = 0;
        for (const auto& req : requests)
        {
            RGResourceID texResID = m_idGenerator.Generate();

            auto resDesc = CreateTextureCubeDesc(*req.asset);
            auto texRes = m_resFactory.CreateTextureResource(resDesc);

            auto& textureCubeResource = req.resource;
            textureCubeResource->Set(texRes);
            textureCubeResource->SetSize(Core::Size{ req.asset->width, req.asset->height });
            CreateTextureCubeViews(m_descFactory, textureCubeResource.get());

            offset = Core::AlignUp(offset, TextureAlignment::Placement);
            uploads.push_back({ texResID, textureCubeResource, req.asset, offset });

            // 6면 * mipCount 개의 subresource 크기 합산
            offset += m_resFactory.GetRequiredIntermediateSize(
                resDesc, 0, req.asset->mipCount * req.asset->faceCount, offset);
        }

        outTotalUploadSize = Core::AlignUp(offset, TextureAlignment::Placement);
        return uploads;
    }

    std::shared_ptr<ResourceContext> CreateResourceContext(
        std::shared_ptr<std::vector<TextureCubeUploadEntry>> textureUploads,
        RGResourceID uploadResID,
        std::size_t totalUploadSize)
    {
        auto* rawContext = new ResourceContext(m_idGenerator.Count());

        for (const auto& upload : *textureUploads)
            rawContext->Set(upload.resID, upload.resource->Get());
        rawContext->Set(uploadResID, m_resFactory.CreateResource(totalUploadSize, ResInitType::Upload));

        // GPU 작업 완료 후 TaskScheduler가 컨텍스트를 놓는 시점에 finalize.
        return std::shared_ptr<ResourceContext>(
            rawContext,
            [this, textureUploads](ResourceContext* ctx) mutable
            {
                FinalizeTextureCubes(*textureUploads);
                delete ctx;
            });
    }

    void BuildUploadPass(
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

    void FinalizeTextureCubes(std::vector<TextureCubeUploadEntry>& textureUploads)
    {
        for (auto& tex : textureUploads)
            tex.resource->MarkReady();
    }

    TaskScheduler& m_taskScheduler;
    DescriptorFactory& m_descFactory;
    ResourceFactory& m_resFactory;

    RenderGraph m_graph;
    RGResourceIDGenerator m_idGenerator;
};