module;

#include "d3dx12.h"

module DxRender.Allocator;

import :FrameConstant;
import Core.Assert;
import DxRender.Definition;

void FrameConstantAllocator::Reset(std::uint32_t slot)
{
    Core::Assert(slot < FrameBufferCount);

    m_slotBaseOffset = m_perSlotSize * slot;
    m_offset = 0;
}

void FrameConstantAllocator::CreateBuffer(Device& device, UINT bufferSize)
{
    Core::Assert(bufferSize > 0);

    m_resource = device.CreateResource(
        CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_RESOURCE_STATE_GENERIC_READ);

    auto hr = m_resource->Map(0, nullptr, reinterpret_cast<void**>(&m_mapped));
    Core::Assert(SUCCEEDED(hr));
}