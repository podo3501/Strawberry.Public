module;

#include <d3d12.h>
#include <wrl/client.h>

export module DxRender.Allocator:BindlessDescriptor;

import std;
import DxRender.Core;
import DxRender.Constants;
import Core.IndexAllocator;

export class BindlessDescriptorAllocator
{
public:
    ~BindlessDescriptorAllocator();
    BindlessDescriptorAllocator() = default;

    BindlessDescriptorAllocator(const BindlessDescriptorAllocator&) = delete;
    BindlessDescriptorAllocator& operator=(const BindlessDescriptorAllocator&) = delete;
    BindlessDescriptorAllocator(BindlessDescriptorAllocator&&) noexcept = default;
    BindlessDescriptorAllocator& operator=(BindlessDescriptorAllocator&&) noexcept = default;

    bool Initialize(Device& device) noexcept;
    UINT AllocatePersistent() noexcept;
    UINT AllocateDynamic() noexcept; // 임시 할당. 프레임 단위가 아니라 fence에 따라 다름. 예약된 고정 크기가 있다.( ex. mipmap 같이 잠시 계산때 쓰고 버리는 거)
    UINT AllocateTransient(std::uint32_t slot, UINT count = 1) noexcept; // 프레임 끝나면 Free. 임시 할당

    void FreeDynamic(UINT index) noexcept;
    void ResetTransient(std::uint32_t slot) noexcept;
    void ResetAll() noexcept;

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT index) const noexcept;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT index) const noexcept;
    ID3D12DescriptorHeap* GetHeap() const noexcept { return m_heap.Get(); }
    UINT GetDescriptorSize() const noexcept { return m_descriptorSize; }

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
    UINT m_descriptorSize{ 0 };

    Core::LinearIndexAllocator m_persistentRegion; // [0, persistentCapacity), 해제 없음
    Core::ReusableIndexAllocator m_dynamicRegion; // [dynamicOffset, dynamicOffset + dynamicCount), 해제 가능
    UINT m_dynamicOffset{ 0 };

    // transient 영역 : [transientOffset, transientOffset + capacity * FrameBufferCount), 프레임당 전체 해제
    std::array<Core::LinearIndexAllocator, FrameBufferCount> m_transientRegion;
    UINT m_transientOffset{ 0 };

    D3D12_CPU_DESCRIPTOR_HANDLE m_cpuStart{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_gpuStart{};
};