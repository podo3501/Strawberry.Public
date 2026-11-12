module;

#include "d3dx12.h"

module DxRender.Allocator;

import :FrameConstant;
import Core.Assert;
import Core.Logger;
import DxRender.Definition;

void FrameConstantAllocator::Reset(std::uint32_t slot)
{
    Core::Assert(slot < FrameBufferCount);

    m_slotBaseOffset = m_perSlotSize * slot;
    m_offset = 0;
}

bool FrameConstantAllocator::CreateBuffer(Device& device, UINT bufferSize)
{
    Core::Assert(bufferSize > 0);

    m_resource = device.CreateResource(
        CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_RESOURCE_STATE_GENERIC_READ);
    if (!m_resource)
        return false;

    auto hr = m_resource->Map(0, nullptr, reinterpret_cast<void**>(&m_mapped));
    if (FAILED(hr))
    {
        Core::Logger::Error("FrameConstantAllocator::CreateBuffer: 버퍼를 매핑하지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return false;
    }
    return true;
}