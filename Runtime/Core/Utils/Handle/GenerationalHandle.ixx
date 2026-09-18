export module Core.GenerationalHandle;

import std;

namespace Core
{
    // FixedHandlePool 전방 선언
    export template<typename T, typename Tag, std::size_t MaxCount>
        class FixedHandlePool;

    export template<typename Tag>
        class GenerationalHandle
    {
    public:
        constexpr GenerationalHandle() = default;

        constexpr std::uint16_t Index() const noexcept { return static_cast<std::uint16_t>(m_value & 0xFFFF); }
        constexpr std::uint16_t Generation() const noexcept { return static_cast<std::uint16_t>((m_value >> 16) & 0xFFFF); }

        constexpr explicit operator bool() const noexcept { return m_value != 0; }

        static constexpr GenerationalHandle Invalid() noexcept { return GenerationalHandle{ 0 }; }

        bool operator==(const GenerationalHandle&) const = default;
        bool operator!=(const GenerationalHandle&) const = default;

    private:
        constexpr explicit GenerationalHandle(std::uint32_t v) : m_value(v) {}

        static constexpr GenerationalHandle Make(std::uint16_t index, std::uint16_t gen)
        {
            return GenerationalHandle{
                static_cast<std::uint32_t>(index) |
                (static_cast<std::uint32_t>(gen) << 16)
            };
        }

        std::uint32_t m_value{ 0 };

        template<typename T, typename Tag2, std::size_t MaxCount>
        friend class FixedHandlePool;
    };
}