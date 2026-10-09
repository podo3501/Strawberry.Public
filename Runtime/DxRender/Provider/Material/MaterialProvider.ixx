export module DxRender.Provider:Material;

import std;
import :IUpdatable;
import :PendingLoadQueue;
import :BuiltinTexture;
import :Texture;
import DxRender.Resource;
import DxRender.Release;
import Core.Assert;
import Core.TypeHierarchy;
import Contract.Asset.Data;
import Contract.Render.Interfaces;

export class MaterialProvider : public IResourceProvider
{
public:
    virtual ~MaterialProvider() override = default;

    MaterialProvider(
        PendingLoadQueue& pendingLoad,
        DeferredReleaser& deferredReleaser,
        TextureProvider& texProvider) noexcept :
        m_pendingLoad{ pendingLoad },
        m_deferredReleaser{ deferredReleaser },
        m_texProvider{ texProvider }
    {}

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<AssetData> asset) override
    {
        if (!asset)
            return BuildPhongResource(nullptr, nullptr, PhongSurface{});

        auto matAsset = Core::Cast<MaterialAsset>(asset);
        if (!matAsset)
            return nullptr;

        switch (matAsset->type)
        {
        case MaterialType::PBR:
        {
            auto pbrAsset = Core::Cast<PbrMaterialAsset>(asset);
            if (!pbrAsset) return nullptr;
            return BuildPbrResource(pbrAsset->albedo, pbrAsset->normal, pbrAsset->arm, pbrAsset->surface);
        }
        case MaterialType::Phong:
        {
            auto phongAsset = Core::Cast<PhongMaterialAsset>(asset);
            if (!phongAsset) return nullptr;
            return BuildPhongResource(phongAsset->albedo, phongAsset->normal, phongAsset->surface);
        }
        }
        Core::Assert(false);

        return nullptr;
    }

    virtual void ReleaseResource(std::shared_ptr<IResource> res) override
    {
        m_deferredReleaser.Add(std::move(res));
    }

private:
    std::shared_ptr<IResource> BuildPbrResource(
        std::shared_ptr<TextureAsset> albedoAsset,
        std::shared_ptr<TextureAsset> normalAsset,
        std::shared_ptr<TextureAsset> armAsset,
        const PbrSurface& surface)
    {
        auto albedoRes = CreateTexResourceOrFallback(albedoAsset, BuiltinTextureType::White);
        if (!albedoRes)
            return nullptr;

        auto normalRes = CreateTexResourceOrFallback(normalAsset, BuiltinTextureType::FlatNormal);
        auto armRes = CreateTexResourceOrFallback(armAsset, BuiltinTextureType::DefaultARM);

        auto pbrRes = std::make_shared<PbrMaterialResource>();
        pbrRes->SetAlbedo(albedoRes);
        pbrRes->SetNormal(normalRes);
        pbrRes->SetArm(armRes);
        pbrRes->SetSurface(surface);

        m_pendingLoad.Add(pbrRes);
        return pbrRes;
    }

    std::shared_ptr<IResource> BuildPhongResource(
        std::shared_ptr<TextureAsset> albedoAsset,
        std::shared_ptr<TextureAsset> normalAsset,
        const PhongSurface& surface)
    {
        auto albedoRes = CreateTexResourceOrFallback(albedoAsset, BuiltinTextureType::White);
        if (!albedoRes)
            return nullptr;

        auto normalRes = CreateTexResourceOrFallback(normalAsset, BuiltinTextureType::FlatNormal);

        auto phongRes = std::make_shared<PhongMaterialResource>();
        phongRes->SetAlbedo(albedoRes);
        phongRes->SetNormal(normalRes);
        phongRes->SetSurface(surface);

        m_pendingLoad.Add(phongRes);
        return phongRes;
    }

    std::shared_ptr<TextureResource> CreateTexResource(std::shared_ptr<TextureAsset> texAsset)
    {
        if (!texAsset) return nullptr;

        auto res = m_texProvider.CreateResource();
        if (!res) return nullptr;

        if (!m_texProvider.LoadResource(res, texAsset))
            return nullptr;

        return res;
    }

    std::shared_ptr<TextureResource> CreateTexResourceOrFallback(
        std::shared_ptr<TextureAsset> texAsset,
        BuiltinTextureType fallbackType)
    {
        if (auto res = CreateTexResource(texAsset))
            return res;

        return m_texProvider.GetBuiltinTexture(fallbackType);
    }

    PendingLoadQueue& m_pendingLoad;
    DeferredReleaser& m_deferredReleaser;
    TextureProvider& m_texProvider;
};