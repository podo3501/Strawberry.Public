export module DxRender.Provider:FrameUploadPools;

import std;
import :FrameUploadPoolType;
import Core.Utils;
import DxRender.Core;
import DxRender.Allocator;

export struct FrameUploadPoolDesc
{
    FrameUploadPoolType type;
    std::uint32_t bufferSizeInBytes{ 0 };
    std::uint32_t elementStride{ 0 };
};

export class FrameUploadPools
{
public:
    FrameUploadPools() = default;
    ~FrameUploadPools() = default;

    FrameUploadPools(const FrameUploadPools&) = delete;
    FrameUploadPools& operator=(const FrameUploadPools&) = delete;

    bool Initialize(Device& device, std::span<const FrameUploadPoolDesc> poolDescs);
    UploadAllocation Allocate(FrameUploadPoolType type, std::uint32_t elementCount) noexcept;
    void Reset(std::uint32_t slot) noexcept;

private:
    FrameUploadAllocator& Get(FrameUploadPoolType type) noexcept;

    std::array<FrameUploadAllocator, Core::EnumSize<FrameUploadPoolType>> m_pools;
};