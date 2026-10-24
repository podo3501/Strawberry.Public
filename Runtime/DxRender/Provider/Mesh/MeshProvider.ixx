export module DxRender.Provider:Mesh;

import std;
import :IUpdatable;
import :MeshLoadRequest;
import :MeshCreateGraphBuilder;
import :PendingUploadQueue;
import DxRender.Release;
import DxRender.Task;
import DxRender.Factory;
import Contract.Common.Asset;
import Contract.Render.Backend;

export class MeshProvider : public IResourceProvider, public IUpdatableProvider
{
public:
    ~MeshProvider() override = default;
    MeshProvider() = delete;
    MeshProvider(
        DeferredReleaser& deferredReleaser,
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory) noexcept;

    std::shared_ptr<IResource> CreateResource(std::shared_ptr<Asset> asset) override;
    void ReleaseResource(std::shared_ptr<IResource> res) override;
    void Update(float avgGpuMs) override;

private:
    DeferredReleaser& m_deferredReleaser;
    MeshCreateGraphBuilder m_createBuilder;
    PendingUploadQueue<MeshLoadRequest> m_pendingLoads; //그래프 콜백에서 직접 MarkReady
};