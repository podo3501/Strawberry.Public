module;

#include <d3d12.h>
#include <wrl/client.h>

export module DxRender.Allocator:BindlessDescriptor;

import std;
import DxRender.Core;
import DxRender.Constants;
import Core.Assert;
import Core.IndexAllocator;
import Contract.Render.Definition;

namespace BindlessDescriptors
{
    export constexpr std::uint32_t MaxCount = 524288;          // 최대 1,000,000개 미만 제한
    export constexpr std::uint32_t PersistentCount = 393216;   // [0, persistentCount) - 해제 없는 영구 할당 (텍스처/머티리얼 등)
    export constexpr std::uint32_t DynamicCount = 32768;       // [persistentCount, persistentCount + dynamicCount) - 개별 free 가능, fence 기반
    export constexpr std::uint32_t TransientCount = 32768;     // 슬롯 하나 크기. 실제 사용량은 TransientCount * FrameBufferCount

    export constexpr std::uint32_t Count = PersistentCount + DynamicCount + TransientCount * FrameBufferCount;

    static_assert(Count <= MaxCount, "BindlessDescriptorConfig: persistent + dynamic + transient*FrameBufferCount가 최대치를 초과했습니다.");
}

export class BindlessDescriptorAllocator
{
public:
    ~BindlessDescriptorAllocator()
    {
        Core::Assert(!m_dynamicRegion.HasOutstanding()); // 작업중인 것이 없어야 한다.
    }

    BindlessDescriptorAllocator() = default;

    BindlessDescriptorAllocator(const BindlessDescriptorAllocator&) = delete;
    BindlessDescriptorAllocator& operator=(const BindlessDescriptorAllocator&) = delete;
    BindlessDescriptorAllocator(BindlessDescriptorAllocator&&) noexcept = default;
    BindlessDescriptorAllocator& operator=(BindlessDescriptorAllocator&&) noexcept = default;

    bool Initialize(Device& device) noexcept
    {
        m_persistentRegion.Initialize(BindlessDescriptors::PersistentCount);

        m_dynamicOffset = BindlessDescriptors::PersistentCount;
        m_dynamicRegion.Initialize(BindlessDescriptors::DynamicCount);

        m_transientOffset = m_dynamicOffset + BindlessDescriptors::DynamicCount;
        for (auto& region : m_transientRegion)
            region.Initialize(BindlessDescriptors::TransientCount); // 슬롯당 capacity는 여기서만 넘겨줌

        m_heap = device.CreateDescriptorHeap(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            BindlessDescriptors::Count,
            D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        if (!m_heap)
            return false;

        m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_cpuStart = m_heap->GetCPUDescriptorHandleForHeapStart();
        m_gpuStart = m_heap->GetGPUDescriptorHandleForHeapStart();
        return true;
    }

    UINT AllocatePersistent() noexcept
    {
        return m_persistentRegion.Allocate();
    }

    UINT AllocateDynamic() noexcept // 임시 할당. 프레임 단위가 아니라 fence에 따라 다름. 예약된 고정 크기가 있다.( ex. mipmap 같이 잠시 계산때 쓰고 버리는 거)
    {
        UINT local = m_dynamicRegion.Allocate();
        if (local == Core::InvalidIndex)
            return UINT_MAX;

        return m_dynamicOffset + local; // 로컬 인덱스를 전역 heap 인덱스로 변환(그냥 앞에 공간 더함)
    }

    UINT AllocateTransient(std::uint32_t slot, UINT count = 1) noexcept // 프레임 끝나면 Free. 임시 할당
    {
        Core::Assert(slot < FrameBufferCount);

        UINT local = m_transientRegion[slot].Allocate(count);
        if (local == Core::InvalidIndex)
            return UINT_MAX;

        UINT slotBase = m_transientOffset + m_transientRegion[slot].Capacity() * slot;
        return slotBase + local;
    }

    void FreeDynamic(UINT index) noexcept
    {
        if (index == UINT_MAX) return;
        m_dynamicRegion.Free(index - m_dynamicOffset); // 전역 인덱스를 로컬 인덱스로 변환(그냥 앞에 공간 뺌)
    }

    void ResetTransient(std::uint32_t slot) noexcept
    {
        Core::Assert(slot < FrameBufferCount);
        m_transientRegion[slot].Reset();
    }

    void ResetAll() noexcept
    {
        m_persistentRegion.Reset();
        m_dynamicRegion.Reset();
        for (auto& region : m_transientRegion)
            region.Reset();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT index) const noexcept
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_cpuStart;
        handle.ptr += static_cast<SIZE_T>(index) * m_descriptorSize;
        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT index) const noexcept
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = m_gpuStart;
        handle.ptr += static_cast<SIZE_T>(index) * m_descriptorSize;
        return handle;
    }

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