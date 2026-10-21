export module DxRender.Provider:TextureCubeCreateGraphBuilder;

import std;
import :TextureCubeLoadRequest;
import DxRender.Resource;
import DxRender.Graph;
import DxRender.Task;
import DxRender.Factory;
import Contract.Render.Assets;

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
        DescriptorFactory& descFactory);

    void LoadTextureCubes(const std::vector<TextureCubeLoadRequest>& requests);

private:
    std::vector<TextureCubeUploadEntry> BuildTextureCubeUploads(
        const std::vector<TextureCubeLoadRequest>& requests,
        std::size_t& outTotalUploadSize);

    std::shared_ptr<ResourceContext> CreateResourceContext(
        std::shared_ptr<std::vector<TextureCubeUploadEntry>> textureUploads,
        RGResourceID uploadResID,
        std::size_t totalUploadSize);

    void BuildUploadPass(
        std::shared_ptr<std::vector<TextureCubeUploadEntry>> textureUploads,
        RGResourceID uploadResID);

    void FinalizeTextureCubes(std::vector<TextureCubeUploadEntry>& textureUploads);

    TaskScheduler& m_taskScheduler;
    DescriptorFactory& m_descFactory;
    ResourceFactory& m_resFactory;

    RenderGraph m_graph;
    RGResourceIDGenerator m_idGenerator;
};