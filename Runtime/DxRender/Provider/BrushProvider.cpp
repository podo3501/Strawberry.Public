module DxRender.Provider;

import :Brush;
import :BuiltinTexture;
import DxRender.Resource;
import Core.Logger;
import Core.TypeHierarchy;

BrushProvider::BrushProvider(
    PendingLoadQueue& pendingLoad,
    DeferredReleaser& deferredReleaser,
    TextureProvider& texProvider) noexcept :
    m_pendingLoad{ pendingLoad },
    m_deferredReleaser{ deferredReleaser },
    m_texProvider{ texProvider }
{}

std::shared_ptr<IResource> BrushProvider::CreateResource(std::shared_ptr<Asset> asset)
{
    auto brushRes = std::make_shared<BrushResource>();

    if (asset)
    {
        auto texRes = m_texProvider.CreateResource();
        if (!texRes) return nullptr;

        auto texAsset = Core::Cast<TextureAsset>(asset);
        if (!texAsset)
        {
            Core::Logger::Error("BrushProvider::CreateResource: 텍스처 에셋이 아니다");
            return nullptr;
        }

        if (!m_texProvider.LoadResource(texRes, texAsset))
            return nullptr; // 원인은 TextureProvider가 남김

        brushRes->SetTexture(texRes);
    }
    else
    {
        auto tex = m_texProvider.GetBuiltinTexture(BuiltinTextureType::White);
        if (!tex)
        {
            Core::Logger::Error("BrushProvider::CreateResource: 기본 흰색 텍스처가 준비되지 않았다");
            return nullptr;
        }
        brushRes->SetTexture(tex);
    }

    m_pendingLoad.Add(brushRes);
    return brushRes;
}

void BrushProvider::ReleaseResource(std::shared_ptr<IResource> res)
{
    m_deferredReleaser.Add(std::move(res));
}