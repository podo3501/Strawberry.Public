export module Runtime.Render.Provider:TransientMesh;

import std;
import :FrameUploadPools;
import Runtime.Render.Core;
import Runtime.Render.Allocator;
import Runtime.Render.Factory;
import Runtime.Render.Resource;
import Core.Assert;
import Client.Render.Definition;

export class TransientMeshProvider
{
public:
    ~TransientMeshProvider() = default;

    explicit TransientMeshProvider(DescriptorFactory& descriptorFactory) noexcept :
        m_descriptorFactory{ descriptorFactory }
    {}

    bool Initialize(Device& device)
    {
        FrameUploadPoolDesc poolDescs[] =
        {
            { FrameUploadPoolType::UIVertex, 16 * 1024 * 1024, sizeof(UIVertex) },
            { FrameUploadPoolType::UIIndex,  16 * 1024 * 1024, sizeof(std::uint32_t) },
        };

        return m_frameUploadPools.Initialize(device, poolDescs);
    }

    std::shared_ptr<TransientMeshResource> Create(
        std::span<const UIVertex> vertices,
        std::span<const std::uint32_t> indices)
    {
        if (vertices.empty() || indices.empty())
            return nullptr;

        auto resource = std::make_shared<TransientMeshResource>();

        UploadAllocation vertex = m_frameUploadPools.Allocate(
            FrameUploadPoolType::UIVertex,
            static_cast<std::uint32_t>(vertices.size()));

        UploadAllocation index = m_frameUploadPools.Allocate(
            FrameUploadPoolType::UIIndex,
            static_cast<std::uint32_t>(indices.size()));

        // 검증: offset이 각 원소 크기의 배수가 아니면 FirstElement 계산이 틀어짐
        Core::Assert(vertex.offset % sizeof(UIVertex) == 0);
        Core::Assert(index.offset % sizeof(std::uint32_t) == 0);

        std::memcpy(vertex.cpuAddress, vertices.data(), vertex.sizeInBytes);
        std::memcpy(index.cpuAddress, indices.data(), index.sizeInBytes);

        constexpr auto invalidIndex = std::numeric_limits<std::uint32_t>::max();

        std::uint32_t vertexHeapIndex =
            m_descriptorFactory.CreateBufferSRV(
                DescriptorAllocationType::Transient,
                *vertex.resource,
                static_cast<std::uint32_t>(vertex.offset / sizeof(UIVertex)),
                static_cast<std::uint32_t>(vertices.size()),
                sizeof(UIVertex));
        if (vertexHeapIndex == invalidIndex)
            return nullptr;

        std::uint32_t indexHeapIndex =
            m_descriptorFactory.CreateBufferSRV(
                DescriptorAllocationType::Transient,
                *index.resource,
                static_cast<std::uint32_t>(index.offset / sizeof(std::uint32_t)),
                static_cast<std::uint32_t>(indices.size()),
                sizeof(std::uint32_t));
        if (indexHeapIndex == invalidIndex)
            return nullptr;

        resource->Initialize(
            static_cast<std::uint32_t>(vertices.size()),
            static_cast<std::uint32_t>(indices.size()),
            vertexHeapIndex,
            indexHeapIndex);

        return resource;
    }

    void ResetFrame(std::uint32_t slot) noexcept
    {
        m_frameUploadPools.Reset(slot);
    }

private:
    FrameUploadPools m_frameUploadPools;
    DescriptorFactory& m_descriptorFactory;
};