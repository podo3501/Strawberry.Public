export module DxRender.Provider:Environment;

import std;
import :IUpdatable;
import :PendingLoadQueue;
import :TextureCube;
import DxRender.Release;
import DxRender.Resource;
import Contract.Render.Assets;
import Contract.Render.Backend;

export class EnvironmentProvider : public IResourceProvider
{
public:
    ~EnvironmentProvider() override = default;
    EnvironmentProvider(
        PendingLoadQueue& pendingLoad,
        DeferredReleaser& deferredReleaser,
        TextureCubeProvider& cubeProvider) noexcept;

    std::shared_ptr<IResource> CreateResource(std::shared_ptr<Asset> asset) override;
    void ReleaseResource(std::shared_ptr<IResource> res) override;

private:
    std::shared_ptr<TextureCubeResource> CreateCubeResource(std::shared_ptr<TextureCubeAsset> cubeAsset);

    PendingLoadQueue& m_pendingLoad;
    DeferredReleaser& m_deferredReleaser;
    TextureCubeProvider& m_cubeProvider;
};