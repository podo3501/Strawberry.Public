export module DxRender.RGResourceID:Allocator;

import std;
import :Types;
import Core.IndexAllocator;

// 리소스 ID 용량 정의
static constexpr std::uint32_t PersistentResourceIDCapacity = 150;
static constexpr std::uint32_t DynamicResourceIDCapacity = 50;
static constexpr std::uint32_t TransientResourceIDCapacity = 100;
export constexpr std::uint32_t TotalResourceIDCapacity =
    PersistentResourceIDCapacity + DynamicResourceIDCapacity + TransientResourceIDCapacity;

export class RGResourceIDAllocator
{
public:
    RGResourceIDAllocator() noexcept
    {
        m_persistentRegion.Initialize(PersistentResourceIDCapacity); // persistent: [0, PersistentResourceIDCapacity)

        m_dynamicOffset = PersistentResourceIDCapacity;
        m_dynamicRegion.Initialize(DynamicResourceIDCapacity); // dynamic: [dynamicOffset, dynamicOffset + DynamicResourceIDCapacity)

        m_transientOffset = m_dynamicOffset + DynamicResourceIDCapacity;
        m_transientRegion.Initialize(TransientResourceIDCapacity); // transient: [transientOffset, transientOffset + TransientResourceIDCapacity)
    }

    ~RGResourceIDAllocator() = default;

    RGResourceID AllocatePersistent() noexcept
    {
        return m_persistentRegion.Allocate();
    }

    RGResourceID AllocateDynamic() noexcept
    {
        Core::Index local = m_dynamicRegion.Allocate();
        if (local == Core::InvalidIndex)
            return InvalidRGID;

        return m_dynamicOffset + local; // 전역 인덱스로 변환
    }

    RGResourceID AllocateTransient() noexcept
    {
        RGResourceID local = m_transientRegion.Allocate();
        if (local == Core::InvalidIndex)
            return InvalidRGID;

        return m_transientOffset + local;
    }

    void FreeDynamic(RGResourceID id) noexcept
    {
        if (id == InvalidRGID) return;
        m_dynamicRegion.Free(id - m_dynamicOffset); // 전역 -> 로컬
    }

    void ResetTransient() noexcept
    {
        m_transientRegion.Reset();
    }

    void ResetAll() noexcept
    {
        m_persistentRegion.Reset();
        m_dynamicRegion.Reset();
        m_transientRegion.Reset();
    }

private:
    Core::LinearIndexAllocator m_persistentRegion; // [0, persistentCount), 해제 없음
    Core::ReusableIndexAllocator m_dynamicRegion;          // [dynamicOffset, dynamicOffset + dynamicCount), 해제 가능
    std::uint32_t m_dynamicOffset{ 0 };

    Core::LinearIndexAllocator m_transientRegion;  // [transientOffset, transientOffset + transientCount), 그래프 빌드마다 리셋
    std::uint32_t m_transientOffset{ 0 };
};