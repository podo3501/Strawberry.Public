export module DxRender.Provider:Texture;

import std;
import :IUpdatable;
import :BuiltinTexture;
import :PendingUploadQueue;
import :TextureCreateGraphBuilder;
import :TextureLoadRequest;
import DxRender.Resource;
import DxRender.Task;
import DxRender.Factory;
import DxRender.Shader;
import DxRender.Core;
import Core.Math;
import Core.Utils;
import Contract.Asset.Data;

export class TextureProvider : public IUpdatableProvider
{
public:
    virtual ~TextureProvider() override = default;
    TextureProvider() = delete;
    TextureProvider(
        Device& device,
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory) noexcept;
    virtual void Update(float avgGpuMs) override;

    std::shared_ptr<TextureResource> CreateResource();
    bool LoadResource(std::shared_ptr<TextureResource> resource, std::shared_ptr<TextureAsset> asset);
    bool Initialize(ShaderLibrary& shaderLibrary);
    std::shared_ptr<TextureResource> GetBuiltinTexture(BuiltinTextureType type) const;

private:
    bool CreateBuiltinTextures();
    std::shared_ptr<TextureResource> CreateBuiltinTexture(std::shared_ptr<TextureAsset> asset);

    TextureCreateGraphBuilder m_createBuilder;
    PendingUploadQueue<TextureLoadRequest> m_pendingLoads;
    std::array<std::shared_ptr<TextureResource>, Core::EnumSize<BuiltinTextureType>> m_builtinTextures;
};