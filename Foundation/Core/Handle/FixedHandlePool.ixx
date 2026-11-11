export module Core.Handle:FixedHandlePool;

import std;
import :GenerationalHandle;
import Core.Logger;

namespace Core
{
    // 최대 MaxCount개까지 들어가는 핸들 풀 (힙 할당 없음). generation이 16비트라서 한 자리를 65,535번 다시 쓰면 옛 핸들이 다시 유효해 보인다
    export template<typename T, typename Tag, std::size_t MaxCount>
        class FixedHandlePool
    {
        static_assert(MaxCount < std::numeric_limits<std::uint16_t>::max(), "인덱스가 uint16_t라서 dummy를 더한 개수가 넘치면 안 된다");

    public:
        using Handle = GenerationalHandle<Tag>;

        FixedHandlePool() = default;

        // 시작할 때 한 번만 부른다. 두 번째 호출이나 MaxCount를 넘는 개수는 오류를 남기고 false다.
        [[nodiscard]] bool Setup(std::size_t maxCount)
        {
            if (m_isSetup)
            {
                Logger::Error("FixedHandlePool::Setup을 두 번 불렀다");
                return false;
            }
            if (maxCount > MaxCount)
            {
                Logger::Error("FixedHandlePool::Setup의 개수 {}가 최대 {}를 넘는다", maxCount, MaxCount);
                return false;
            }

            m_isSetup = true;
            m_capacity = static_cast<std::uint16_t>(maxCount + 1); // 내부 capacity = usable + dummy
            RebuildFreeList();
            return true;
        }

        template<typename... Args>
        Handle Emplace(Args&&... args)
        {
            if (m_freeCount == 0)
                return Handle::Invalid();

            const std::uint16_t index = m_freeList[--m_freeCount];

            auto& slot = m_slots[index];
            slot.data = T(std::forward<Args>(args)...);
            slot.alive = true;

            return Handle::Make(index, slot.generation);
        }

        bool Remove(Handle h)
        {
            if (!IsValid(h)) return false;

            ReleaseSlot(h.Index());
            m_freeList[m_freeCount++] = h.Index();
            return true;
        }

        void Clear()
        {
            for (std::uint16_t i = 1; i < m_capacity; ++i)
            {
                if (m_slots[i].alive)
                    ReleaseSlot(i);
            }

            RebuildFreeList();
        }

        auto* Find(this auto& self, Handle h)
        {
            return self.IsValid(h) ? &self.m_slots[h.Index()].data : nullptr;
        }

        bool IsValid(Handle h) const
        {
            if (!h) return false;

            std::uint16_t idx = h.Index();
            if (idx == 0 || idx >= m_capacity) return false;

            const auto& slot = m_slots[idx];
            return slot.alive && slot.generation == h.Generation();
        }

        std::size_t AliveCount() const { return static_cast<std::size_t>(m_capacity - 1) - m_freeCount; }

        // 살아 있는 항목마다 f(핸들, 값)을 부른다
        template<typename Func>
        void Visit(this auto& self, Func&& f)
        {
            for (std::uint16_t i = 1; i < self.m_capacity; ++i)
            {
                auto& slot = self.m_slots[i];
                if (slot.alive)
                    f(Handle::Make(i, slot.generation), slot.data);
            }
        }

    private:
        struct Slot
        {
            T data{};
            std::uint16_t generation = 1;
            bool alive = false;
        };

        // 값을 비우고 generation을 올려 옛 핸들을 무효로 한다
        void ReleaseSlot(std::uint16_t index)
        {
            auto& slot = m_slots[index];
            slot.data = T{}; // 지우는 순간 객체를 비운다 (HandlePool과 같다)
            slot.alive = false;

            ++slot.generation;
            if (slot.generation == 0) slot.generation = 1; // generation 증가 (0 방지)
        }

        void RebuildFreeList()
        {
            m_freeCount = 0;
            for (std::uint16_t i = 1; i < m_capacity; ++i)
                m_freeList[m_freeCount++] = i; // index 1 ~ maxCount 사용 가능
        }

        std::array<Slot, MaxCount + 1> m_slots{}; // +1 해서 index 0 dummy 확보
        std::array<std::uint16_t, MaxCount> m_freeList{};
        std::size_t m_freeCount{ 0 };
        std::uint16_t m_capacity{ 1 }; // 항상 최소 1 (dummy만 있는 상태)
        bool m_isSetup{ false };
    };
}
