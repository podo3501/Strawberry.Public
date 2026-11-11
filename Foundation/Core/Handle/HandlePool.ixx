export module Core.Handle:HandlePool;

import std;
import :IDHandle;
import Core.Assert;

namespace Core
{
    // 넣는 개수에 제한이 없는 핸들 풀. 지운 핸들은 generation이 달라져서 옛 핸들로는 새 값을 찾지 못한다.
    // Visit 도중에는 Remove만 할 수 있고 Emplace는 Assert다 (슬롯 벡터가 재할당되기 때문이다)
    export template<typename T, typename Tag>
        class HandlePool
    {
    public:
        using Handle = IDHandle<Tag>;

        HandlePool()
        {
            m_slots.emplace_back(); // index 0은 dummy를 하나 넣어서 절대 0 인덱스가 생성되지 않게 함
        }

        template<typename... Args>
        Handle Emplace(Args&&... args)
        {
            Assert(m_visitDepth == 0);

            std::uint32_t index;
            if (!m_freeList.empty())
            {
                index = m_freeList.back();
                m_freeList.pop_back();
            }
            else
            {
                index = static_cast<std::uint32_t>(m_slots.size());
                m_slots.emplace_back();
            }

            auto& slot = m_slots[index];
            slot.data.emplace(std::forward<Args>(args)...);

            ++m_aliveCount;
            return Handle{ index, slot.generation }; // generation은 1 이상 보장됨
        }

        bool Remove(Handle h)
        {
            if (!IsValid(h)) return false;

            ReleaseSlot(h.index);
            m_freeList.push_back(h.index);
            --m_aliveCount;
            return true;
        }

        void Clear()
        {
            for (std::uint32_t i = 1; i < m_slots.size(); ++i) // 0은 dummy
            {
                if (m_slots[i].data.has_value())
                    ReleaseSlot(i);
            }

            m_freeList.clear();
            for (std::uint32_t i = 1; i < m_slots.size(); ++i)
                m_freeList.push_back(i);

            m_aliveCount = 0;
        }

        auto* Find(this auto& self, Handle h)
        {
            return self.IsValid(h) ? &*self.m_slots[h.index].data : nullptr;
        }

        bool IsValid(Handle h) const
        {
            if (!h) return false;
            if (h.index >= m_slots.size()) return false;

            const auto& slot = m_slots[h.index];
            return slot.data.has_value() && slot.generation == h.generation;
        }

        std::size_t AliveCount() const { return m_aliveCount; }
        bool Empty() const { return m_aliveCount == 0; }

        // 살아 있는 항목마다 f(핸들, 값)을 부른다
        template<typename Func>
        void Visit(this auto& self, Func&& f)
        {
            ++self.m_visitDepth;
            for (std::uint32_t i = 1; i < self.m_slots.size(); ++i)
            {
                auto& slot = self.m_slots[i];
                if (slot.data.has_value())
                    f(Handle{ i, slot.generation }, *slot.data);
            }
            --self.m_visitDepth;
        }

    private:
        struct Slot
        {
            std::optional<T> data;        // 값이 있으면 살아 있는 슬롯이다
            std::uint32_t generation = 1; // 0은 사용 안 함
        };

        // 값을 비우고 generation을 올려 옛 핸들을 무효로 한다
        void ReleaseSlot(std::uint32_t index)
        {
            auto& slot = m_slots[index];
            slot.data.reset();

            ++slot.generation;
            if (slot.generation == 0) slot.generation = 1; // generation 증가 (0 방지)
        }

        std::vector<Slot> m_slots;
        std::vector<std::uint32_t> m_freeList;
        std::size_t m_aliveCount{ 0 };
        mutable std::uint32_t m_visitDepth{ 0 };
    };
}
