export module DxRender.TransientMesh:TransientMesh;

import std;
import :FrameUploadPools;
import DxRender.Core;
import DxRender.Factory;
import DxRender.Resource;
import Contract.Render.Backend.Types;

export class TransientMesh
{
public:
    ~TransientMesh() = default;
    explicit TransientMesh(DescriptorFactory& descriptorFactory) noexcept;

    bool Initialize(Device& device);

    std::shared_ptr<TransientMeshResource> Create(
        const std::vector<UIVertex>& vertices,
        const std::vector<std::uint32_t>& indices);

    void ResetFrame(std::uint32_t slot) noexcept;

private:
    FrameUploadPools m_frameUploadPools;
    DescriptorFactory& m_descriptorFactory;
};