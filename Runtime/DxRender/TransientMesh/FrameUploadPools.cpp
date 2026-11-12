module DxRender.TransientMesh;

import :FrameUploadPools;
import Core.Logger;
import Core.Utils;

bool FrameUploadPools::Initialize(Device& device, const std::vector<FrameUploadPoolDesc>& poolDescs)
{
    if (poolDescs.empty())
    {
        Core::Logger::Error("FrameUploadPools::Initialize: 등록할 풀이 하나도 없다");
        return false;
    }

    for (const auto& desc : poolDescs)
    {
        std::size_t index = static_cast<std::size_t>(desc.type);
        if (index >= Core::EnumSize<FrameUploadPoolType>)
        {
            Core::Logger::Error("FrameUploadPools::Initialize: 알 수 없는 풀 종류다 ({})", index);
            return false;
        }

        if (m_pools[index].IsInitialized()) // 같은 종류를 두 번 등록하지 못한다
        {
            Core::Logger::Error("FrameUploadPools::Initialize: 풀 종류 {}를 이미 등록했다", index);
            return false;
        }

        if (!m_pools[index].Initialize(device, desc.bufferSizeInBytes, desc.elementStride))
            return false;
    }

    return true;
}

UploadAllocation FrameUploadPools::Allocate(FrameUploadPoolType type, std::uint32_t elementCount) noexcept
{
    FrameUploadAllocator* pool = Find(type);
    if (!pool)
        return {};

    return pool->Allocate(elementCount);
}

void FrameUploadPools::Reset(std::uint32_t slot) noexcept
{
    for (auto& pool : m_pools)
    {
        if (pool.IsInitialized())
            pool.Reset(slot);
    }
}

FrameUploadAllocator* FrameUploadPools::Find(FrameUploadPoolType type) noexcept
{
    std::size_t index = static_cast<std::size_t>(type);
    if (index >= Core::EnumSize<FrameUploadPoolType>)
    {
        Core::Logger::Error("FrameUploadPools: 알 수 없는 풀 종류다 ({})", index);
        return nullptr;
    }

    if (!m_pools[index].IsInitialized()) // 등록하지 않은 종류를 요청했다
    {
        Core::Logger::Error("FrameUploadPools: 풀 종류 {}를 등록하지 않았다", index);
        return nullptr;
    }

    return &m_pools[index];
}