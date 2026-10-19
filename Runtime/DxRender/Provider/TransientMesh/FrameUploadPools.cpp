module DxRender.Provider;

import :FrameUploadPools;
import Core.Assert;
import Core.Utils;

bool FrameUploadPools::Initialize(Device& device, const std::vector<FrameUploadPoolDesc>& poolDescs)
{
    for (const auto& desc : poolDescs)
    {
        std::size_t index = static_cast<std::size_t>(desc.type);
        Core::Assert(index < Core::EnumSize<FrameUploadPoolType>);
        Core::Assert(!m_pools[index].IsInitialized()); // 같은 타입 중복 등록 방지

        if (!m_pools[index].Initialize(device, desc.bufferSizeInBytes, desc.elementStride))
            return false;
    }

    return true;
}

UploadAllocation FrameUploadPools::Allocate(FrameUploadPoolType type, std::uint32_t elementCount) noexcept
{
    return Get(type).Allocate(elementCount);
}

void FrameUploadPools::Reset(std::uint32_t slot) noexcept
{
    for (auto& pool : m_pools)
    {
        if (pool.IsInitialized())
            pool.Reset(slot);
    }
}

FrameUploadAllocator& FrameUploadPools::Get(FrameUploadPoolType type) noexcept
{
    std::size_t index = static_cast<std::size_t>(type);
    Core::Assert(index < Core::EnumSize<FrameUploadPoolType>);
    Core::Assert(m_pools[index].IsInitialized()); // 등록 안 된 타입을 실수로 요청하는 걸 여기서 잡음

    return m_pools[index];
}