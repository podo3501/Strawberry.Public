export module DxRender.Provider:Mesh;

import std;
import :IUpdatable;
import :MeshLoadRequest;
import :MeshCreateGraphBuilder;
import :PendingUploadQueue;
import :Budget;
import DxRender.Resource;
import DxRender.Release;
import DxRender.Constants;
import DxRender.Task;
import DxRender.Factory;
import Core.Bit;
import Core.TypeHierarchy;
import Contract.Asset.Data;
import Contract.Render.Interfaces;

std::pair<std::size_t, std::size_t> EstimateBytes(const MeshAsset* mesh)
{
    if (!mesh) return { 0, 0 };

    std::size_t vb = mesh->vertices.size();
    std::size_t ib = mesh->indices.size() * sizeof(std::uint32_t);

    vb = Core::AlignUp(vb, BufferAlignment::VertexBuffer);
    ib = Core::AlignUp(ib, BufferAlignment::IndexBuffer);

    return { vb, ib };
}

export class MeshProvider : public IResourceProvider, public IUpdatableProvider
{
public:
    virtual ~MeshProvider() override = default;
    MeshProvider() = delete;

    MeshProvider(
        DeferredReleaser& deferredReleaser,
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory) noexcept :
        m_deferredReleaser{ deferredReleaser },
        m_createBuilder{ taskScheduler, resFactory, descFactory }
    {}

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<AssetData> asset) override
    {
        if (!asset) return nullptr;

        auto meshAsset = Core::Cast<MeshAsset>(asset);
        if (!meshAsset) return nullptr;

        auto [vbBytes, ibBytes] = EstimateBytes(meshAsset.get());
        auto meshRes = std::make_shared<StaticMeshResource>();

        MeshLoadRequest req;
        req.resource = meshRes;
        req.asset = meshAsset;
        req.vbBytes = vbBytes;
        req.ibBytes = ibBytes;
        req.estimatedBytes = vbBytes + ibBytes;

        m_pendingLoads.Push(req);
        return meshRes;
    }

    virtual void ReleaseResource(std::shared_ptr<IResource> res) override
    {
        m_deferredReleaser.Add(std::move(res));
    }

    virtual void Update(float avgGpuMs) override
    {
        auto uploadBudgetBytes = ComputeBudget(avgGpuMs, ProviderBudget::Mesh);
        m_pendingLoads.Flush(uploadBudgetBytes, [this](std::vector<MeshLoadRequest>& batch) {
            m_createBuilder.LoadMeshes(batch);
            });
    }

private:
    DeferredReleaser& m_deferredReleaser;
    MeshCreateGraphBuilder m_createBuilder;
    PendingUploadQueue<MeshLoadRequest> m_pendingLoads; //그래프 콜백에서 직접 MarkReady
};