export module Core.IndexAllocator:ReusableIndexAllocator;

import std;
import Core.BasicTypes;
import Core.Logger;

namespace Core
{
    export class ReusableIndexAllocator
    {
    public:
        ReusableIndexAllocator() = default;

        // 시작할 때 한 번만 부른다 (다른 스레드가 쓰기 전). 두 번째 호출은 오류를 남기고 false다
        [[nodiscard]] bool Initialize(Index capacity) noexcept
        {
            if (m_isInitialized)
            {
                Logger::Error("ReusableIndexAllocator::Initialize를 두 번 불렀다");
                return false;
            }

            m_isInitialized = true;
            m_capacity = capacity;
            Reset();
            return true;
        }

        // 용량이 차면 오류를 남기고 InvalidIndex이다
        Index Allocate() noexcept
        {
            std::lock_guard lock(m_mutex);
            if (!m_freeList.empty())
            {
                Index index = m_freeList.back();
                m_freeList.pop_back();
                m_isFree[index] = false;
                return index;
            }
            if (m_allocated >= m_capacity)
            {
                Logger::Error("ReusableIndexAllocator: 용량 {}을 모두 사용했다", m_capacity);
                return InvalidIndex;
            }
            m_isFree.push_back(false);
            return m_allocated++;
        }

        // 할당한 적 없거나 이미 해제한 인덱스는 오류를 남기고 무시한다 (같은 인덱스가 두 번 나가지 않게). InvalidIndex는 조용히 무시한다
        void Free(Index index) noexcept
        {
            if (index == InvalidIndex) return;

            std::lock_guard lock(m_mutex);
            if (index >= m_allocated)
            {
                Logger::Error("ReusableIndexAllocator: 할당한 적 없는 인덱스 {}를 해제했다", index);
                return;
            }
            if (m_isFree[index])
            {
                Logger::Error("ReusableIndexAllocator: 이미 해제한 인덱스 {}를 다시 해제했다", index);
                return;
            }

            m_isFree[index] = true;
            m_freeList.push_back(index);
        }

        void Reset() noexcept
        {
            std::lock_guard lock(m_mutex);

            m_allocated = 0;
            m_freeList.clear();
            m_isFree.clear();
        }

        bool HasOutstanding() const noexcept
        {
            std::lock_guard lock(m_mutex);
            return m_allocated - static_cast<Index>(m_freeList.size()) > 0;
        }

        Index Capacity() const noexcept { return m_capacity; }

    private:
        Index m_capacity{ 0 };
        Index m_allocated{ 0 };
        std::vector<Index> m_freeList;
        std::vector<bool> m_isFree; // 인덱스마다 지금 비어 있는지 (m_allocated 크기). 이중 해제를 막는다
        mutable std::mutex m_mutex;
        bool m_isInitialized{ false };
    };
}
