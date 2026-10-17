export module DxRender.Provider:Environment;

import std;
import :IUpdatable;
import :PendingLoadQueue;
import :TextureCube;
import DxRender.Release;
import DxRender.Resource;
import Contract.Asset.Data;
import Contract.Render.Interfaces;

export class EnvironmentProvider : public IResourceProvider
{
public:
    virtual ~EnvironmentProvider() override = default;
    EnvironmentProvider(
        PendingLoadQueue& pendingLoad,
        DeferredReleaser& deferredReleaser,
        TextureCubeProvider& cubeProvider) noexcept;

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<AssetData> asset) override;
    virtual void ReleaseResource(std::shared_ptr<IResource> res) override;

private:
    std::shared_ptr<TextureCubeResource> CreateCubeResource(std::shared_ptr<TextureCubeAsset> cubeAsset);

    PendingLoadQueue& m_pendingLoad;
    DeferredReleaser& m_deferredReleaser;
    TextureCubeProvider& m_cubeProvider;
};