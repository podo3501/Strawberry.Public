export module DxRender.Provider:TextureCreateGraphBuilder;

import std;
import :MipGenerator;
import :TextureLoadRequest;
import DxRender.Graph;
import DxRender.Resource;
import DxRender.Task;
import DxRender.Factory;
import DxRender.Shader;
import DxRender.Definition;
import DxRender.Command;
import Contract.Asset.Data;

struct TextureUploadEntry
{
    RGResourceID resID;
    std::shared_ptr<TextureResource> resource;
    std::shared_ptr<TextureAsset> asset;

    std::size_t offset{ 0 };
    bool generateMips{ false };

    std::vector<std::uint32_t> mipSrvIndices;
    std::vector<std::uint32_t> mipUavIndices;
};

export class TextureCreateGraphBuilder
{
public:
    ~TextureCreateGraphBuilder() = default;
    TextureCreateGraphBuilder() = delete;
    TextureCreateGraphBuilder(
        Device& device,
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory);

    bool Initialize(ShaderLibrary& shaderLibrary);
    void LoadTextures(const std::vector<TextureLoadRequest>& requests);

private:
    std::vector<TextureUploadEntry> BuildTextureUploads(
        const std::vector<TextureLoadRequest>& requests,
        std::size_t& outTotalUploadSize,
        bool& outHasMipTask);

    std::shared_ptr<ResourceContext> CreateResourceContext(
        std::shared_ptr<std::vector<TextureUploadEntry>> textureUploads,
        RGResourceID uploadResID,
        std::size_t totalUploadSize);

    void BuildUploadPass(
        std::shared_ptr<std::vector<TextureUploadEntry>> textureUploads,
        RGResourceID uploadResID);

    void BuildMipPass(
        std::shared_ptr<std::vector<TextureUploadEntry>> textureUploads);

    void FinalizeTextures(std::vector<TextureUploadEntry>& textureUploads);

    TaskScheduler& m_taskScheduler;
    MipGenerator m_mipGenerator;
    DescriptorFactory& m_descFactory;
    ResourceFactory& m_resFactory;

    RGResourceIDGenerator m_idGenerator;
    RenderGraph m_graph;
};