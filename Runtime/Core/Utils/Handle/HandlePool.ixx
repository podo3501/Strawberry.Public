export module Core.HandlePool;

import std;
import Core.Handle;

namespace Core
{
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
            slot.data.reset();
            slot.handle = Handle{ index, slot.generation };
            slot.data.emplace(std::forward<Args>(args)...);
            slot.alive = true;

            ++m_aliveCount;
            return slot.handle; // generation은 1 이상 보장됨
        }

        bool Remove(Handle h)
        {
            if (!IsValid(h)) return false;

            auto& slot = m_slots[h.index];
            slot.data.reset();
            slot.handle = Handle::Invalid();
            slot.alive = false;

            slot.generation++;
            if (slot.generation == 0) slot.generation = 1; // generation 증가 (0 방지)

            m_freeList.push_back(h.index);
            --m_aliveCount;

            return true;
        }

        void Clear()
        {
            for (std::size_t i = 1; i < m_slots.size(); ++i) // 0은 dummy
            {
                auto& slot = m_slots[i];

                if (!slot.alive) continue;

                slot.data.reset();
                slot.handle = Handle::Invalid();
                slot.alive = false;

                slot.generation++;
                if (slot.generation == 0) slot.generation = 1;
            }

            m_freeList.clear();
            for (std::size_t i = 1; i < m_slots.size(); ++i)
                m_freeList.push_back(static_cast<std::uint32_t>(i));

            m_aliveCount = 0;
        }

        T* Find(Handle h)
        {
            if (!IsValid(h)) return nullptr;

            auto& slot = m_slots[h.index];
            if (!slot.data.has_value())
                return nullptr;

            return &(*slot.data);
        }

        const T* Find(Handle h) const
        {
            if (!IsValid(h)) return nullptr;

            const auto& slot = m_slots[h.index];
            if (!slot.data.has_value())
                return nullptr;

            return &(*slot.data);
        }

        bool IsValid(Handle h) const
        {
            if (!h) return false;
            if (h.index >= m_slots.size()) return false;

            const auto& slot = m_slots[h.index];
            return slot.alive && slot.generation == h.generation;
        }

        std::size_t Size() const { return m_slots.size(); }
        std::size_t AliveCount() const { return m_aliveCount; }
        bool Empty() const { return m_aliveCount == 0; }

        template<typename Func>
        void Visit(Func&& f)
        {
            for (auto& slot : m_slots)
            {
                if (!slot.alive) continue;
                f(slot.handle, *slot.data);
            }
        }

        template<typename Func>
        void Visit(Func&& f) const
        {
            for (const auto& slot : m_slots)
            {
                if (!slot.alive) continue;
                f(slot.handle, *slot.data);
            }
        }

    private:
        template<typename ValueType>
        struct HandleSlot
        {
            std::optional<ValueType> data;
            Handle handle{ Handle::Invalid() };
            std::uint32_t generation = 1; // 0은 사용 안 함
            bool alive = false;
        };

        std::vector<HandleSlot<T>> m_slots;
        std::vector<std::uint32_t> m_freeList;
        std::size_t m_aliveCount{ 0 };
    };
}