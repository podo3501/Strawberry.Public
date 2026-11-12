export module DxRender.TransientMesh:FrameUploadPools;

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

    bool Initialize(Device& device, const std::vector<FrameUploadPoolDesc>& poolDescs);
    UploadAllocation Allocate(FrameUploadPoolType type, std::uint32_t elementCount) noexcept;
    void Reset(std::uint32_t slot) noexcept;

private:
    FrameUploadAllocator* Find(FrameUploadPoolType type) noexcept; // 범위 밖이거나 등록하지 않은 종류면 오류를 남기고 nullptr

    std::array<FrameUploadAllocator, Core::EnumSize<FrameUploadPoolType>> m_pools;
};