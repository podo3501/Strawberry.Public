module;

#include <d3d12.h>
#include <wrl.h>
#include "d3dx12.h"

export module Runtime.Render.Allocator:FrameConstant;

import std;
import Core.Assert;
import Core.Bit;
import Runtime.Render.Core;
import Runtime.Render.Constants;

export class FrameConstantAllocator
{
public:
    ~FrameConstantAllocator() = default;
    FrameConstantAllocator() = default;

    FrameConstantAllocator(const FrameConstantAllocator&) = delete;
    FrameConstantAllocator& operator=(const FrameConstantAllocator&) = delete;
    FrameConstantAllocator(FrameConstantAllocator&&) noexcept = default;
    FrameConstantAllocator& operator=(FrameConstantAllocator&&) noexcept = default;

    template<typename T>
    void Initialize(Device& device, UINT count)
    {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable.");
        Core::Assert(count > 0);

        m_stride = static_cast<UINT>(Core::AlignUp(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));
        m_perSlotSize = m_stride * count;

        CreateBuffer(device, m_perSlotSize * FrameBufferCount);
    }

    template<typename T>
    D3D12_GPU_VIRTUAL_ADDRESS AllocateConstant(const T& data)
    {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable.");

        UINT localOffset = m_offset.fetch_add(m_stride, std::memory_order_relaxed);
        UINT offset = m_slotBaseOffset + localOffset;

        Core::Assert(localOffset + m_stride <= m_perSlotSize);
        Core::Assert(sizeof(T) <= m_stride);

        std::memcpy(m_mapped + offset, &data, sizeof(T));

        return m_resource->GetGPUVirtualAddress() + offset;
    }

    void Reset(std::uint32_t slot)
    {
        Core::Assert(slot < FrameBufferCount);

        m_slotBaseOffset = m_perSlotSize * slot;
        m_offset = 0;
    }

private:
    void CreateBuffer(Device& device, UINT bufferSize)
    {
        Core::Assert(bufferSize > 0);

        m_resource = device.CreateResource(
            CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_HEAP_TYPE_UPLOAD,
            D3D12_RESOURCE_STATE_GENERIC_READ);

        auto hr = m_resource->Map(0, nullptr, reinterpret_cast<void**>(&m_mapped));
        Core::Assert(SUCCEEDED(hr));
    }

private:
    Resource m_resource;
    std::uint8_t* m_mapped{ nullptr };
    std::atomic<UINT> m_offset{ 0 };
    UINT m_stride{ 0 };
    UINT m_perSlotSize{ 0 };
    UINT m_slotBaseOffset{ 0 };
};