export module DxRender.RGResourceID:Allocator;

import std;
import :Types;
import Core.IndexAllocator;

// 리소스 ID 용량 정의
constexpr std::uint32_t PersistentResourceIDCapacity = 150;
constexpr std::uint32_t DynamicResourceIDCapacity = 50;
constexpr std::uint32_t TransientResourceIDCapacity = 100;
export constexpr std::uint32_t TotalResourceIDCapacity =
PersistentResourceIDCapacity + DynamicResourceIDCapacity + TransientResourceIDCapacity;

export class RGResourceIDAllocator
{
public:
    ~RGResourceIDAllocator() = default;
    RGResourceIDAllocator() noexcept;

    RGResourceID AllocatePersistent() noexcept;
    RGResourceID AllocateDynamic() noexcept;
    RGResourceID AllocateTransient() noexcept;

    void FreeDynamic(RGResourceID id) noexcept;
    void ResetTransient() noexcept;
    void ResetAll() noexcept;

private:
    Core::LinearIndexAllocator m_persistentRegion; // [0, persistentCount), 해제 없음
    Core::ReusableIndexAllocator m_dynamicRegion; // [dynamicOffset, dynamicOffset + dynamicCount), 해제 가능
    std::uint32_t m_dynamicOffset{ 0 };

    Core::LinearIndexAllocator m_transientRegion; // [transientOffset, transientOffset + transientCount), 그래프 빌드마다 리셋
    std::uint32_t m_transientOffset{ 0 };
};