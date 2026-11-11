module DxRender.RGResourceID;

import :Allocator;
import Core.Assert;
import Core.Logger;

RGResourceIDAllocator::RGResourceIDAllocator() noexcept
{
    // 만들 때 한 번만 초기화하므로 실패하지 않는다
    const bool persistentOk = m_persistentRegion.Initialize(PersistentResourceIDCapacity); // persistent: [0, PersistentResourceIDCapacity)

    m_dynamicOffset = PersistentResourceIDCapacity;
    const bool dynamicOk = m_dynamicRegion.Initialize(DynamicResourceIDCapacity); // dynamic: [dynamicOffset, dynamicOffset + DynamicResourceIDCapacity)

    m_transientOffset = m_dynamicOffset + DynamicResourceIDCapacity;
    const bool transientOk = m_transientRegion.Initialize(TransientResourceIDCapacity); // transient: [transientOffset, transientOffset + TransientResourceIDCapacity)
    Core::Assert(persistentOk && dynamicOk && transientOk);
}

RGResourceID RGResourceIDAllocator::AllocatePersistent() noexcept
{
    return m_persistentRegion.Allocate();
}

RGResourceID RGResourceIDAllocator::AllocateDynamic() noexcept
{
    Core::Index local = m_dynamicRegion.Allocate();
    if (local == Core::InvalidIndex)
        return InvalidRGID;

    return m_dynamicOffset + local; // 전역 인덱스로 변환
}

RGResourceID RGResourceIDAllocator::AllocateTransient() noexcept
{
    RGResourceID local = m_transientRegion.Allocate();
    if (local == Core::InvalidIndex)
        return InvalidRGID;

    return m_transientOffset + local;
}

void RGResourceIDAllocator::FreeDynamic(RGResourceID id) noexcept
{
    if (id == InvalidRGID) return;

    if (id < m_dynamicOffset || id - m_dynamicOffset >= DynamicResourceIDCapacity)
    {
        Core::Logger::Error("RGResourceIDAllocator::FreeDynamic: dynamic 구역이 아닌 ID {}를 해제했다", id);
        return;
    }

    m_dynamicRegion.Free(id - m_dynamicOffset); // 전역 -> 로컬
}

void RGResourceIDAllocator::ResetTransient() noexcept
{
    m_transientRegion.Reset();
}

void RGResourceIDAllocator::ResetAll() noexcept
{
    m_persistentRegion.Reset();
    m_dynamicRegion.Reset();
    m_transientRegion.Reset();
}