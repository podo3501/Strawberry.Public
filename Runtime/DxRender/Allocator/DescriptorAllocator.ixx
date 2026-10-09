module;

#include <d3d12.h>
#include <wrl/client.h>

export module DxRender.Allocator:Descriptor;

import std;
import Core.Assert;
import Core.IndexAllocator;
import DxRender.Core;

export class DescriptorAllocator
{
public:
    ~DescriptorAllocator() = default;
    DescriptorAllocator() = default;

    DescriptorAllocator(const DescriptorAllocator&) = delete;
    DescriptorAllocator& operator=(const DescriptorAllocator&) = delete;
    DescriptorAllocator(DescriptorAllocator&&) noexcept = default;
    DescriptorAllocator& operator=(DescriptorAllocator&&) noexcept = default;

    bool Initialize(Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT capacity) noexcept
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

    UINT Allocate() noexcept { return m_indexAllocator.Allocate(); }

    // 주의! Pending 해제되어 GPU로부터 안전해진 핸들만 Free 해야 합니다.
    void Free(UINT index) noexcept { m_indexAllocator.Free(index); }
    void Reset() noexcept { m_indexAllocator.Reset(); }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT index) const noexcept
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_cpuStart;
        handle.ptr += static_cast<SIZE_T>(index) * m_descriptorSize;
        return handle;
    }

    ID3D12DescriptorHeap* GetHeap() const noexcept { return m_heap.Get(); }
    UINT GetDescriptorSize() const noexcept { return m_descriptorSize; }

private:
    Core::ReusableIndexAllocator m_indexAllocator;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;

    D3D12_CPU_DESCRIPTOR_HANDLE m_cpuStart{};
    UINT m_descriptorSize{ 0 };
};