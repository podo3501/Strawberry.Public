export module Core.DualIndexAllocator;

import std;
import Core.Assert;
import Core.BasicTypes;

namespace Core
{
    export class DualIndexAllocator
    {
    public:
        constexpr DualIndexAllocator() = default;

        void Initialize(Index capacity) noexcept
        {
            m_capacity = capacity;
            m_front = 0;
            m_back = 0;
        }

        Index AllocateFront() noexcept
        {
            if (m_front + m_back >= m_capacity)
            {
                Assert(false);
                return InvalidIndex;
            }
            return m_front++;
        }

        // count개의 연속 슬롯 시작 인덱스를 반환 (뒤에서부터 채움)
        Index AllocateBack(Index count) noexcept
        {
            Index used = m_back + count;
            if (used > m_capacity - m_front)
            {
                Assert(false);
                return InvalidIndex;
            }
            m_back = used;
            return m_capacity - used;
        }

        void ResetBack() noexcept { m_back = 0; }
        void ResetAll() noexcept { m_front = 0; m_back = 0; }

        Index Capacity() const noexcept { return m_capacity; }

    private:
        Index m_capacity = 0;
        Index m_front = 0;
        Index m_back = 0;
    };
}