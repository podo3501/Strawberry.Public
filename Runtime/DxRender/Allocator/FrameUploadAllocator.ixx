module;

#include <wrl.h>
#include "d3dx12.h"

export module DxRender.Allocator:FrameUpload;

import std;
import :UploadAllocation;
import Core.Assert;
import Core.Bit;
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

    bool Initialize(
        Device& device,
        UINT bufferSizeInBytes,
        UINT elementStride)
    {
        Core::Assert(bufferSizeInBytes > 0);
        Core::Assert(elementStride > 0);

        m_elementStride = elementStride;
        m_perSlotSize = Core::AlignUpGeneric(bufferSizeInBytes, elementStride);

        UINT slotCapacityInElements = m_perSlotSize / elementStride;
        for (auto& region : m_region)
            region.Initialize(slotCapacityInElements);

        m_resource = device.CreateResource(
            CD3DX12_RESOURCE_DESC::Buffer(m_perSlotSize * FrameBufferCount), // 슬롯 수만큼 버퍼를 늘려서, 슬롯마다 겹치지 않는 영역을 갖게 한다.
            D3D12_HEAP_TYPE_UPLOAD,
            D3D12_RESOURCE_STATE_GENERIC_READ);

        auto hr = m_resource->Map(0, nullptr, reinterpret_cast<void**>(&m_mapped));
        Core::Assert(SUCCEEDED(hr));

        return true;
    }

    UploadAllocation Allocate(UINT elementCount) noexcept
    {
        Core::Assert(m_elementStride > 0); // Initialize 등록 없이 Allocate 호출 금지

        UINT localElement = m_region[m_currentSlot].Allocate(elementCount);
        Core::Assert(localElement != Core::InvalidIndex); // capacity 초과 - bufferSizeInBytes 확장 필요

        UINT sizeInBytes = elementCount * m_elementStride;
        UINT globalOffset = m_perSlotSize * m_currentSlot + localElement * m_elementStride; // slot offset + element offset

        UploadAllocation alloc;
        alloc.resource = &m_resource;
        alloc.cpuAddress = m_mapped + globalOffset;
        alloc.gpuAddress = m_resource->GetGPUVirtualAddress() + globalOffset;
        alloc.offset = globalOffset;
        alloc.sizeInBytes = sizeInBytes;

        return alloc;
    }

    void Reset(std::uint32_t slot)
    {
        Core::Assert(slot < FrameBufferCount);

        m_currentSlot = slot;
        m_region[slot].Reset();
    }

    bool IsInitialized() const noexcept { return m_perSlotSize > 0; }

private:
    Resource m_resource;
    UINT m_elementStride{ 0 };
    UINT m_perSlotSize{ 0 };
    std::uint8_t* m_mapped{ nullptr };

    std::array<Core::LinearIndexAllocator, FrameBufferCount> m_region;
    std::uint32_t m_currentSlot{ 0 };
};