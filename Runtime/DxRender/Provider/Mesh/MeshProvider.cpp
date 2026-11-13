module DxRender.Provider;

import std;
import :Mesh;
import :Budget;
import DxRender.Resource;
import DxRender.Constants;
import Core.Bit;
import Core.Logger;
import Core.TypeHierarchy;
import Contract.Render.Assets;

static std::pair<std::size_t, std::size_t> EstimateBytes(const MeshAsset* mesh)
{
    if (!mesh) return { 0, 0 };

    std::size_t vb = mesh->vertices.size();
    std::size_t ib = mesh->indices.size() * sizeof(std::uint32_t);

    vb = Core::AlignUp(vb, BufferAlignment::VertexBuffer);
    ib = Core::AlignUp(ib, BufferAlignment::IndexBuffer);

    return { vb, ib };
}

// 정점과 인덱스가 업로드할 수 있는 모양인지 확인한다 (아니면 이유와 함께 Error)
static bool IsValidMeshAsset(const MeshAsset& mesh)
{
    if (mesh.vertexStride == 0 || mesh.vertexCount == 0 || mesh.indices.empty())
    {
        Core::Logger::Error("MeshProvider: 메시 에셋에 정점이나 인덱스가 없다 (정점 {}, 보폭 {}, 인덱스 {})",
            mesh.vertexCount, mesh.vertexStride, mesh.indices.size());
        return false;
    }

    const auto expectedBytes = static_cast<std::size_t>(mesh.vertexCount) * mesh.vertexStride;
    if (mesh.vertices.size() != expectedBytes)
    {
        Core::Logger::Error("MeshProvider: 메시 에셋의 정점 바이트 수가 정점 수 x 보폭과 다르다 ({} != {})",
            mesh.vertices.size(), expectedBytes);
        return false;
    }
    return true;
}

MeshProvider::MeshProvider(
    DeferredReleaser& deferredReleaser,
    TaskScheduler& taskScheduler,
    ResourceFactory& resFactory,
    DescriptorFactory& descFactory) noexcept :
    m_deferredReleaser{ deferredReleaser },
    m_createBuilder{ taskScheduler, resFactory, descFactory }
{}

std::shared_ptr<IResource> MeshProvider::CreateResource(std::shared_ptr<Asset> asset)
{
    auto meshAsset = Core::Cast<MeshAsset>(asset);
    if (!meshAsset)
    {
        Core::Logger::Error("MeshProvider::CreateResource: 메시 에셋이 아니다 (에셋 {})", asset ? "다른 종류" : "없음");
        return nullptr;
    }
    if (!IsValidMeshAsset(*meshAsset)) return nullptr;

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