module DxRender.TransientMesh;

import :TransientMesh;
import Core.Assert;
import Core.Logger;

TransientMesh::TransientMesh(DescriptorFactory& descriptorFactory) noexcept :
    m_descriptorFactory{ descriptorFactory }
{}

bool TransientMesh::Initialize(Device& device)
{
    std::vector<FrameUploadPoolDesc> poolDescs =
    {
        { FrameUploadPoolType::UIVertex, 16 * 1024 * 1024, sizeof(UIVertex) },
        { FrameUploadPoolType::UIIndex,  16 * 1024 * 1024, sizeof(std::uint32_t) },
    };

    return m_frameUploadPools.Initialize(device, poolDescs);
}

std::shared_ptr<TransientMeshResource> TransientMesh::Create(
    const std::vector<UIVertex>& vertices,
    const std::vector<std::uint32_t>& indices)
{
    if (vertices.empty() || indices.empty())
    {
        Core::Logger::Error("TransientMesh::Create: 정점 {}개, 인덱스 {}개로는 메시를 만들 수 없다", vertices.size(), indices.size());
        return nullptr;
    }

    // 원인은 아래층(업로드 풀, 디스크립터 할당기)이 남기고, 여기서는 어느 단계에서 멈췄는지만 덧붙인다
    UploadAllocation vertex = m_frameUploadPools.Allocate(
        FrameUploadPoolType::UIVertex,
        static_cast<std::uint32_t>(vertices.size()));
    if (!vertex)
    {
        Core::Logger::Error("TransientMesh::Create: 정점 {}개를 올릴 업로드 자리가 없다", vertices.size());
        return nullptr;
    }

    UploadAllocation index = m_frameUploadPools.Allocate(
        FrameUploadPoolType::UIIndex,
        static_cast<std::uint32_t>(indices.size()));
    if (!index)
    {
        Core::Logger::Error("TransientMesh::Create: 인덱스 {}개를 올릴 업로드 자리가 없다", indices.size()); // 정점 구간은 이 프레임이 끝날 때까지 낭비된다
        return nullptr;
    }

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
    {
        Core::Logger::Error("TransientMesh::Create: 정점 {}개의 디스크립터를 만들지 못했다", vertices.size());
        return nullptr;
    }

    std::uint32_t indexHeapIndex =
        m_descriptorFactory.CreateBufferSRV(
            DescriptorAllocationType::Transient,
            *index.resource,
            static_cast<std::uint32_t>(index.offset / sizeof(std::uint32_t)),
            static_cast<std::uint32_t>(indices.size()),
            sizeof(std::uint32_t));
    if (indexHeapIndex == invalidIndex)
    {
        Core::Logger::Error("TransientMesh::Create: 인덱스 {}개의 디스크립터를 만들지 못했다", indices.size());
        return nullptr;
    }

    auto resource = std::make_shared<TransientMeshResource>();
    resource->Initialize(
        static_cast<std::uint32_t>(vertices.size()),
        static_cast<std::uint32_t>(indices.size()),
        vertexHeapIndex,
        indexHeapIndex);

    return resource;
}

void TransientMesh::ResetFrame(std::uint32_t slot) noexcept
{
    m_frameUploadPools.Reset(slot);
}
