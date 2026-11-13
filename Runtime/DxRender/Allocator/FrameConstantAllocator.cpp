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
    m_overflowLogged = false;
}

bool FrameConstantAllocator::InitializeBuffer(Device& device, UINT stride, UINT count)
{
    if (count == 0)
    {
        Core::Logger::Error("FrameConstantAllocator::Initialize: count가 0이다");
        return false;
    }

    const std::uint64_t totalSize = static_cast<std::uint64_t>(stride) * count * FrameBufferCount;
    if (totalSize > std::numeric_limits<UINT>::max())
    {
        Core::Logger::Error("FrameConstantAllocator::Initialize: 버퍼 크기가 32비트를 넘는다 (stride={}, count={}, 슬롯={})",
            stride, count, FrameBufferCount);
        return false;
    }

    m_stride = stride;
    m_perSlotSize = stride * count;

    return CreateBuffer(device, static_cast<UINT>(totalSize));
}

D3D12_GPU_VIRTUAL_ADDRESS FrameConstantAllocator::AllocateBytes(const void* data, std::size_t size)
{
    // 병렬 기록 중 여러 스레드가 함께 부른다. 칸 번호를 받은 뒤에 한도를 확인하므로 넘친 호출은 아무것도 쓰지 않는다.
    const std::uint64_t localOffset = m_offset.fetch_add(m_stride, std::memory_order_relaxed);

    if (localOffset + m_stride > m_perSlotSize)
    {
        if (!m_overflowLogged.exchange(true))
            Core::Logger::Error("FrameConstantAllocator: 상수 버퍼 한도({}개)를 넘었다. 이 프레임의 나머지 항목은 건너뛴다",
                m_perSlotSize / m_stride);
        return InvalidAddress;
    }

    if (size > m_stride)
    {
        if (!m_overflowLogged.exchange(true))
            Core::Logger::Error("FrameConstantAllocator: 자료 크기({})가 칸 크기({})보다 크다 (Initialize한 형식과 다른 형식이다)",
                size, m_stride);
        return InvalidAddress;
    }

    const UINT offset = m_slotBaseOffset + static_cast<UINT>(localOffset);
    std::memcpy(m_mapped + offset, data, size);

    return m_resource->GetGPUVirtualAddress() + offset;
}

bool FrameConstantAllocator::CreateBuffer(Device& device, UINT bufferSize)
{
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