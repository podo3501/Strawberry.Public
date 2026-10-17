export module DxRender.Provider:TransientMesh;

import std;
import :FrameUploadPools;
import DxRender.Core;
import DxRender.Factory;
import DxRender.Resource;
import Contract.Render.Types;

export class TransientMeshProvider
{
public:
    ~TransientMeshProvider() = default;
    explicit TransientMeshProvider(DescriptorFactory& descriptorFactory) noexcept;

    bool Initialize(Device& device);

    std::shared_ptr<TransientMeshResource> Create(
        std::span<const UIVertex> vertices,
        std::span<const std::uint32_t> indices);

    void ResetFrame(std::uint32_t slot) noexcept;

private:
    FrameUploadPools m_frameUploadPools;
    DescriptorFactory& m_descriptorFactory;
};