module;

#include <d3d12.h>

module DxRender.Allocator;

import std;
import :BindlessDescriptor;
import DxRender.Definition;
import Core.Assert;
import Core.Logger;
import Contract.Render.Backend.Types;

namespace BindlessDescriptors
{
    constexpr std::uint32_t MaxCount = 524288; // 최대 1,000,000개 미만 제한
}

BindlessDescriptorAllocator::~BindlessDescriptorAllocator()
{
    Core::Assert(!m_dynamicRegion.HasOutstanding()); // 작업중인 것이 없어야 한다.
}

bool BindlessDescriptorAllocator::Initialize(Device& device, const BindlessDescriptorConfig& config) noexcept
{
    const std::uint64_t total = std::uint64_t{ config.persistentCount } + config.dynamicCount + std::uint64_t{ config.transientCount } * FrameBufferCount;
    if (total > BindlessDescriptors::MaxCount)
    {
        Core::Logger::Error("BindlessDescriptorAllocator::Initialize: 서술자 {}개가 최대치 {}개를 넘는다", total, BindlessDescriptors::MaxCount);
        return false;
    }

    if (!m_persistentRegion.Initialize(config.persistentCount)) // [0, persistentCount)
        return false;

    m_dynamicOffset = config.persistentCount;
    if (!m_dynamicRegion.Initialize(config.dynamicCount))
        return false;

    m_transientOffset = m_dynamicOffset + config.dynamicCount;
    for (auto& region : m_transientRegion)
    {
        if (!region.Initialize(config.transientCount)) // 슬롯당 capacity는 여기서만 넘겨줌
            return false;
    }

    m_heap = device.CreateDescriptorHeap(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        static_cast<UINT>(total),
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    if (!m_heap)
        return false;

    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_cpuStart = m_heap->GetCPUDescriptorHandleForHeapStart();
    m_gpuStart = m_heap->GetGPUDescriptorHandleForHeapStart();
    return true;
}

UINT BindlessDescriptorAllocator::AllocatePersistent() noexcept
{
    return m_persistentRegion.Allocate();
}

UINT BindlessDescriptorAllocator::AllocateDynamic() noexcept // 임시 할당. 프레임 단위가 아니라 fence에 따라 다름. 예약된 고정 크기가 있다.( ex. mipmap 같이 잠시 계산때 쓰고 버리는 거)
{
    UINT local = m_dynamicRegion.Allocate();
    if (local == Core::InvalidIndex)
        return UINT_MAX;

    return m_dynamicOffset + local; // 로컬 인덱스를 전역 heap 인덱스로 변환(그냥 앞에 공간 더함)
}

UINT BindlessDescriptorAllocator::AllocateTransient(std::uint32_t slot, UINT count) noexcept // 프레임 끝나면 Free. 임시 할당
{
    Core::Assert(slot < FrameBufferCount);

    UINT local = m_transientRegion[slot].Allocate(count);
    if (local == Core::InvalidIndex)
        return UINT_MAX;

    UINT slotBase = m_transientOffset + m_transientRegion[slot].Capacity() * slot;
    return slotBase + local;
}

void BindlessDescriptorAllocator::FreeDynamic(UINT index) noexcept
{
    if (index == UINT_MAX) return;
    m_dynamicRegion.Free(index - m_dynamicOffset); // 전역 인덱스를 로컬 인덱스로 변환(그냥 앞에 공간 뺌)
}

void BindlessDescriptorAllocator::ResetTransient(std::uint32_t slot) noexcept
{
    Core::Assert(slot < FrameBufferCount);
    m_transientRegion[slot].Reset();
}

void BindlessDescriptorAllocator::ResetAll() noexcept
{
    m_persistentRegion.Reset();
    m_dynamicRegion.Reset();
    for (auto& region : m_transientRegion)
        region.Reset();
}

D3D12_CPU_DESCRIPTOR_HANDLE BindlessDescriptorAllocator::GetCpuHandle(UINT index) const noexcept
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_cpuStart;
    handle.ptr += static_cast<SIZE_T>(index) * m_descriptorSize;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE BindlessDescriptorAllocator::GetGpuHandle(UINT index) const noexcept
{
    D3D12_GPU_DESCRIPTOR_HANDLE handle = m_gpuStart;
    handle.ptr += static_cast<SIZE_T>(index) * m_descriptorSize;
    return handle;
}