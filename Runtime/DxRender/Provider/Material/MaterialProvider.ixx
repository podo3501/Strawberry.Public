export module DxRender.Provider:Material;

import std;
import :PendingLoadQueue;
import :BuiltinTexture;
import :Texture;
import DxRender.Resource;
import DxRender.Release;
import Contract.Asset.Data;
import Contract.Render.Interfaces;

export class MaterialProvider : public IResourceProvider
{
public:
    virtual ~MaterialProvider() override = default;
    MaterialProvider(
        PendingLoadQueue& pendingLoad,
        DeferredReleaser& deferredReleaser,
        TextureProvider& texProvider) noexcept;

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<AssetData> asset) override;
    virtual void ReleaseResource(std::shared_ptr<IResource> res) override;

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