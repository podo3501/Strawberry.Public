export module Core.IndexAllocator:LinearIndexAllocator;

import std;
import Core.BasicTypes;
import Core.Logger;

namespace Core
{
    export class LinearIndexAllocator
    {
    public:
        constexpr LinearIndexAllocator() = default;

        // atomic 멤버 때문에 복사할 수 없다
        LinearIndexAllocator(const LinearIndexAllocator&) = delete;
        LinearIndexAllocator& operator=(const LinearIndexAllocator&) = delete;

        // 시작할 때 한 번만 부른다 (다른 스레드가 할당하기 전). 두 번째 호출은 오류를 남기고 false다
        [[nodiscard]] bool Initialize(Index capacity) noexcept
        {
            if (m_isInitialized)
            {
                Logger::Error("LinearIndexAllocator::Initialize를 두 번 불렀다");
                return false;
            }

            m_isInitialized = true;
            m_capacity = capacity;
            m_allocated.store(0, std::memory_order_relaxed);
            return true;
        }

        // 연속한 count개의 시작 인덱스를 돌려준다. 0개를 요청하거나 남은 자리가 모자라면 오류를 남기고 InvalidIndex이며, 아무것도 소모하지 않는다
        Index Allocate(Index count = 1) noexcept
        {
            if (count == 0)
            {
                Logger::Error("LinearIndexAllocator: 0개는 할당할 수 없다");
                return InvalidIndex;
            }

            Index current = m_allocated.load(std::memory_order_relaxed);
            do
            {
                if (current > m_capacity || count > m_capacity - current)
                {
                    Logger::Error("LinearIndexAllocator: 용량 {} 중 {}개를 쓰고 있어 {}개를 더 줄 수 없다", m_capacity, current, count);
                    return InvalidIndex;
                }
            } while (!m_allocated.compare_exchange_weak(current, current + count, std::memory_order_relaxed));

            return current;
        }

        // 전체 리소스를 다시 로드하는 등 특수한 경우에만 사용
        void Reset() noexcept
        {
            m_allocated.store(0, std::memory_order_relaxed);
        }

        Index Capacity() const noexcept { return m_capacity; }
        Index Allocated() const noexcept { return m_allocated.load(std::memory_order_relaxed); }

    private:
        Index m_capacity{ 0 };
        std::atomic<Index> m_allocated{ 0 };
        bool m_isInitialized{ false };
    };
}
