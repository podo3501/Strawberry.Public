module DxRender.Provider;

import std;
import :Mesh;
import :Budget;
import DxRender.Resource;
import DxRender.Constants;
import Core.Bit;
import Core.TypeHierarchy;
import Contract.Asset.Data;

static std::pair<std::size_t, std::size_t> EstimateBytes(const MeshAsset* mesh)
{
    if (!mesh) return { 0, 0 };

    std::size_t vb = mesh->vertices.size();
    std::size_t ib = mesh->indices.size() * sizeof(std::uint32_t);

    vb = Core::AlignUp(vb, BufferAlignment::VertexBuffer);
    ib = Core::AlignUp(ib, BufferAlignment::IndexBuffer);

    return { vb, ib };
}

MeshProvider::MeshProvider(
    DeferredReleaser& deferredReleaser,
    TaskScheduler& taskScheduler,
    ResourceFactory& resFactory,
    DescriptorFactory& descFactory) noexcept :
    m_deferredReleaser{ deferredReleaser },
    m_createBuilder{ taskScheduler, resFactory, descFactory }
{}

std::shared_ptr<IResource> MeshProvider::CreateResource(std::shared_ptr<AssetData> asset)
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

void MeshProvider::ReleaseResource(std::shared_ptr<IResource> res)
{
    m_deferredReleaser.Add(std::move(res));
}

void MeshProvider::Update(float avgGpuMs)
{
    auto uploadBudgetBytes = ComputeBudget(avgGpuMs, ProviderBudget::Mesh);
    m_pendingLoads.Flush(uploadBudgetBytes, [this](std::vector<MeshLoadRequest>& batch) {
        m_createBuilder.LoadMeshes(batch);
        });
}