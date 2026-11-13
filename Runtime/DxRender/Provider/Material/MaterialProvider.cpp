module DxRender.Provider;

import :Material;
import DxRender.Resource;
import Core.Logger;
import Core.TypeHierarchy;
import Contract.Render.Assets;

MaterialProvider::MaterialProvider(
    PendingLoadQueue& pendingLoad,
    DeferredReleaser& deferredReleaser,
    TextureProvider& texProvider) noexcept :
    m_pendingLoad{ pendingLoad },
    m_deferredReleaser{ deferredReleaser },
    m_texProvider{ texProvider }
{}

std::shared_ptr<IResource> MaterialProvider::CreateResource(std::shared_ptr<Asset> asset)
{
    if (!asset)
        return BuildPhongResource(nullptr, nullptr, PhongSurface{});

    auto matAsset = Core::Cast<MaterialAsset>(asset);
    if (!matAsset)
    {
        Core::Logger::Error("MaterialProvider::CreateResource: 재질 에셋이 아니다");
        return nullptr;
    }

    switch (matAsset->type)
    {
    case MaterialType::PBR:
    {
        auto pbrAsset = Core::Cast<PbrMaterialAsset>(asset);
        if (!pbrAsset)
        {
            Core::Logger::Error("MaterialProvider::CreateResource: PBR 재질인데 PBR 에셋이 아니다");
            return nullptr;
        }
        return BuildPbrResource(pbrAsset->albedo, pbrAsset->normal, pbrAsset->arm, pbrAsset->surface);
    }
    case MaterialType::Phong:
    {
        auto phongAsset = Core::Cast<PhongMaterialAsset>(asset);
        if (!phongAsset)
        {
            Core::Logger::Error("MaterialProvider::CreateResource: Phong 재질인데 Phong 에셋이 아니다");
            return nullptr;
        }
        return BuildPhongResource(phongAsset->albedo, phongAsset->normal, phongAsset->surface);
    }
    }

    Core::Logger::Error("MaterialProvider::CreateResource: 알 수 없는 재질 종류 {}", static_cast<int>(matAsset->type));
    return nullptr;
}

void MaterialProvider::ReleaseResource(std::shared_ptr<IResource> res)
{
    m_deferredReleaser.Add(std::move(res));
}


std::shared_ptr<IResource> MaterialProvider::BuildPbrResource(
    std::shared_ptr<TextureAsset> albedoAsset,
    std::shared_ptr<TextureAsset> normalAsset,
    std::shared_ptr<TextureAsset> armAsset,
    const PbrSurface& surface)
{
    auto albedoRes = CreateTexResourceOrFallback(albedoAsset, BuiltinTextureType::White);
    auto normalRes = CreateTexResourceOrFallback(normalAsset, BuiltinTextureType::FlatNormal);
    auto armRes = CreateTexResourceOrFallback(armAsset, BuiltinTextureType::DefaultARM);
    if (!albedoRes || !normalRes || !armRes)
    {
        Core::Logger::Error("MaterialProvider::BuildPbrResource: 텍스처를 얻지 못했다 (기본 텍스처가 준비되지 않았을 수 있다)");
        return nullptr;
    }

    auto pbrRes = std::make_shared<PbrMaterialResource>();
    pbrRes->SetAlbedo(albedoRes);
    pbrRes->SetNormal(normalRes);
    pbrRes->SetArm(armRes);
    pbrRes->SetSurface(surface);

    m_pendingLoad.Add(pbrRes);
    return pbrRes;
}

std::shared_ptr<IResource> MaterialProvider::BuildPhongResource(
    std::shared_ptr<TextureAsset> albedoAsset,
    std::shared_ptr<TextureAsset> normalAsset,
    const PhongSurface& surface)
{
    auto albedoRes = CreateTexResourceOrFallback(albedoAsset, BuiltinTextureType::White);
    auto normalRes = CreateTexResourceOrFallback(normalAsset, BuiltinTextureType::FlatNormal);
    if (!albedoRes || !normalRes)
    {
        Core::Logger::Error("MaterialProvider::BuildPhongResource: 텍스처를 얻지 못했다 (기본 텍스처가 준비되지 않았을 수 있다)");
        return nullptr;
    }

    auto phongRes = std::make_shared<PhongMaterialResource>();
    phongRes->SetAlbedo(albedoRes);
    phongRes->SetNormal(normalRes);
    phongRes->SetSurface(surface);

    m_pendingLoad.Add(phongRes);
    return phongRes;
}

std::shared_ptr<TextureResource> MaterialProvider::CreateTexResource(std::shared_ptr<TextureAsset> texAsset)
{
    if (!texAsset) return nullptr;

    auto res = m_texProvider.CreateResource();
    if (!res) return nullptr;

    if (!m_texProvider.LoadResource(res, texAsset))
        return nullptr;

    return res;
}

std::shared_ptr<TextureResource> MaterialProvider::CreateTexResourceOrFallback(
    std::shared_ptr<TextureAsset> texAsset,
    BuiltinTextureType fallbackType)
{
    if (auto res = CreateTexResource(texAsset))
        return res;

    return m_texProvider.GetBuiltinTexture(fallbackType);
}