export module Core.IndexAllocator:Reusable;

import std;
import Core.Assert;
import Core.Types;

namespace Core
{
    export class ReusableIndexAllocator
    {
    public:
        ReusableIndexAllocator() = default;

        void Initialize(Index capacity) noexcept
        {
            m_capacity = capacity;
            m_allocated = 0;
            m_freeList.clear();
        }

        Index Allocate() noexcept
        {
            std::lock_guard lock(m_mutex);
            if (!m_freeList.empty())
            {
                Index index = m_freeList.back();
                m_freeList.pop_back();
                return index;
            }
            if (m_allocated >= m_capacity)
            {
                Assert(false);
                return InvalidIndex;
            }
            return m_allocated++;
        }

        void Free(Index index) noexcept
        {
            if (index == InvalidIndex) return;

            std::lock_guard lock(m_mutex);
            Assert(index < m_allocated);
            m_freeList.push_back(index);
        }

        void Reset() noexcept
        {
            std::lock_guard lock(m_mutex);

            m_allocated = 0;
            m_freeList.clear();
        }

        bool HasOutstanding() noexcept
        {
            std::lock_guard lock(m_mutex);
            return m_allocated - static_cast<Index>(m_freeList.size()) > 0;
        }

        Index Capacity() const noexcept { return m_capacity; }

    private:
        Index m_capacity{ 0 };
        Index m_allocated{ 0 };
        std::vector<Index> m_freeList;
        std::mutex m_mutex;
    };
}