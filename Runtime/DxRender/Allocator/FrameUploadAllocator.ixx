module;

#include <wrl.h>
#include "d3dx12.h"

export module DxRender.Allocator:FrameUpload;

import std;
import :UploadAllocation;
import Core.IndexAllocator;
import DxRender.Core;
import DxRender.Constants;
import DxRender.Resource;

export class FrameUploadAllocator
{
public:
    FrameUploadAllocator() = default;
    ~FrameUploadAllocator() = default;

    FrameUploadAllocator(const FrameUploadAllocator&) = delete;
    FrameUploadAllocator& operator=(const FrameUploadAllocator&) = delete;
    FrameUploadAllocator(FrameUploadAllocator&&) noexcept = default;
    FrameUploadAllocator& operator=(FrameUploadAllocator&&) noexcept = default;

    bool Initialize(Device& device, UINT bufferSizeInBytes, UINT elementStride);
    UploadAllocation Allocate(UINT elementCount) noexcept;
    void Reset(std::uint32_t slot);
    bool IsInitialized() const noexcept { return m_perSlotSize > 0; }

private:
    Resource m_resource;
    UINT m_elementStride{ 0 };
    UINT m_perSlotSize{ 0 };
    std::uint8_t* m_mapped{ nullptr };

    std::array<Core::LinearIndexAllocator, FrameBufferCount> m_region;
    std::uint32_t m_currentSlot{ 0 };
};