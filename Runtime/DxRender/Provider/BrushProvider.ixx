export module DxRender.Provider:Brush;

import std;
import Core.TypeHierarchy;
import Contract.Render.Interfaces;
import :IUpdatable;
import :PendingLoadQueue;
import DxRender.Release;
import :Texture;
import DxRender.Resource;
import :BuiltinTexture;

export class BrushProvider : public IResourceProvider
{
public:
    virtual ~BrushProvider() override = default;
    BrushProvider() = delete;

    BrushProvider(
        PendingLoadQueue& pendingLoad,
        DeferredReleaser& deferredReleaser,
        TextureProvider& texProvider) noexcept :
        m_pendingLoad{ pendingLoad },
        m_deferredReleaser{ deferredReleaser },
        m_texProvider{ texProvider }
    {}

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<AssetData> asset) override
    {
        auto brushRes = std::make_shared<BrushResource>();

        if (asset)
        {
            auto texRes = m_texProvider.CreateResource();
            if (!texRes) return nullptr;

            auto texAsset = Core::Cast<TextureAsset>(asset);
            if (!texAsset) return nullptr;

            if (!m_texProvider.LoadResource(texRes, texAsset))
                return nullptr;

            brushRes->SetTexture(texRes);
        }
        else
        {
            auto tex = m_texProvider.GetBuiltinTexture(BuiltinTextureType::White);
            brushRes->SetTexture(tex);
        }

        m_pendingLoad.Add(brushRes);
        return brushRes;
    }

    virtual void ReleaseResource(std::shared_ptr<IResource> res) override
    {
        m_deferredReleaser.Add(std::move(res));
    }

private:
    PendingLoadQueue& m_pendingLoad;
    DeferredReleaser& m_deferredReleaser;
    TextureProvider& m_texProvider;
};