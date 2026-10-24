export module DxRender.Provider:Material;

import std;
import :PendingLoadQueue;
import :BuiltinTexture;
import :Texture;
import DxRender.Resource;
import DxRender.Release;
import Contract.Render.Assets;
import Contract.Render.Backend;

export class MaterialProvider : public IResourceProvider
{
public:
    ~MaterialProvider() override = default;
    MaterialProvider(
        PendingLoadQueue& pendingLoad,
        DeferredReleaser& deferredReleaser,
        TextureProvider& texProvider) noexcept;

    std::shared_ptr<IResource> CreateResource(std::shared_ptr<Asset> asset) override;
    void ReleaseResource(std::shared_ptr<IResource> res) override;

private:
    std::shared_ptr<IResource> BuildPbrResource(
        std::shared_ptr<TextureAsset> albedoAsset,
        std::shared_ptr<TextureAsset> normalAsset,
        std::shared_ptr<TextureAsset> armAsset,
        const PbrSurface& surface);

    std::shared_ptr<IResource> BuildPhongResource(
        std::shared_ptr<TextureAsset> albedoAsset,
        std::shared_ptr<TextureAsset> normalAsset,
        const PhongSurface& surface);

    std::shared_ptr<TextureResource> CreateTexResource(std::shared_ptr<TextureAsset> texAsset);
    std::shared_ptr<TextureResource> CreateTexResourceOrFallback(
        std::shared_ptr<TextureAsset> texAsset,
        BuiltinTextureType fallbackType);

    PendingLoadQueue& m_pendingLoad;
    DeferredReleaser& m_deferredReleaser;
    TextureProvider& m_texProvider;
};