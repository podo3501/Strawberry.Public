export module DxRender.Provider:Brush;

import std;
import :IUpdatable;
import :PendingLoadQueue;
import :Texture;
import DxRender.Release;
import Contract.Render.Backend;

export class BrushProvider : public IResourceProvider
{
public:
    virtual ~BrushProvider() override = default;
    BrushProvider() = delete;
    BrushProvider(
        PendingLoadQueue& pendingLoad,
        DeferredReleaser& deferredReleaser,
        TextureProvider& texProvider) noexcept;

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<Asset> asset) override;
    virtual void ReleaseResource(std::shared_ptr<IResource> res) override;

private:
    PendingLoadQueue& m_pendingLoad;
    DeferredReleaser& m_deferredReleaser;
    TextureProvider& m_texProvider;
};