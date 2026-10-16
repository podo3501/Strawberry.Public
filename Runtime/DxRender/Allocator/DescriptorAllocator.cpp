module DxRender.Allocator;

import :Descriptor;
import Core.Assert;

bool DescriptorAllocator::Initialize(Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT capacity) noexcept
{
    Core::Assert(type != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    m_indexAllocator.Initialize(capacity);
    m_heap = device.CreateDescriptorHeap(
        type,
        capacity,
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    if (!m_heap)
        return false;

    m_descriptorSize = device->GetDescriptorHandleIncrementSize(type);
    m_cpuStart = m_heap->GetCPUDescriptorHandleForHeapStart();

    return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::GetCpuHandle(UINT index) const noexcept
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_cpuStart;
    handle.ptr += static_cast<SIZE_T>(index) * m_descriptorSize;
    return handle;
}