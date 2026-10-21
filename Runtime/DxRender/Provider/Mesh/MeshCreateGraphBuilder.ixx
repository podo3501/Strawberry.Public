export module DxRender.Provider:MeshCreateGraphBuilder;

import std;
import :MeshLoadRequest;
import :MeshUtils;
import DxRender.Resource;
import DxRender.Graph;
import DxRender.Task;
import DxRender.Factory;
import Contract.Render.Assets;

struct MeshUploadEntry
{
    RGResourceID vbResID{ InvalidRGID };
    RGResourceID ibResID{ InvalidRGID };

    UploadRegion vbRegion;
    UploadRegion ibRegion;

    std::shared_ptr<MeshResource> resource;
    std::shared_ptr<MeshAsset> asset;

    std::uint32_t vbHeapIndex{ std::numeric_limits<std::uint32_t>::max() };
    std::uint32_t ibHeapIndex{ std::numeric_limits<std::uint32_t>::max() };
};

export class MeshCreateGraphBuilder
{
public:
    ~MeshCreateGraphBuilder() = default;
    MeshCreateGraphBuilder() = delete;

    MeshCreateGraphBuilder(
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory);

    void LoadMeshes(const std::vector<MeshLoadRequest>& requests);

private:
    std::vector<MeshUploadEntry> BuildMeshUploads(
        const std::vector<MeshLoadRequest>& requests,
        std::size_t& outTotalUploadSize);

    std::shared_ptr<ResourceContext> CreateResourceContext(
        std::shared_ptr<std::vector<MeshUploadEntry>> meshUploads,
        RGResourceID uploadResID,
        std::size_t totalUploadSize);

    void BuildUploadPass(
        std::shared_ptr<std::vector<MeshUploadEntry>> meshUploads,
        RGResourceID uploadResID);

    void FinalizeMeshes(std::vector<MeshUploadEntry>& meshUploads);

    TaskScheduler& m_taskScheduler;
    ResourceFactory& m_resFactory;
    DescriptorFactory& m_descFactory;

    RenderGraph m_graph;
    RGResourceIDGenerator m_idGenerator;
};