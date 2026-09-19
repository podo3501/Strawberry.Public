module;

#include "Foundation/Preprocessor/Assert.h"

export module Core.LinearIndexAllocator;

import std;
import Core.Types;

namespace Core
{
    export class LinearIndexAllocator
    {
    public:
        constexpr LinearIndexAllocator() = default;

        // atomic 변수는 복사/이동이 불가능하므로 명시적 삭제 또는 default 설정
        LinearIndexAllocator(const LinearIndexAllocator&) = delete;
        LinearIndexAllocator& operator=(const LinearIndexAllocator&) = delete;

        void Initialize(Index capacity) noexcept
        {
            m_capacity = capacity;
            m_allocated.store(0, std::memory_order_relaxed);
        }

        Index Allocate(Index count = 1) noexcept
        {
            Index index = m_allocated.fetch_add(count, std::memory_order_relaxed);
            if (index + count > m_capacity)
            {
                Assert(false);
                return InvalidIndex;
            }
            return index;
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
    };
}