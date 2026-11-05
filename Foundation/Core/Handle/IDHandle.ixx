export module Core.Handle:ID;

import std;

namespace Core
{
    export template<typename Tag>
    struct IDHandle
    {
        std::uint32_t index{ 0 };
        std::uint32_t generation{ 0 };

        constexpr IDHandle() = default;
        constexpr IDHandle(std::uint32_t i, std::uint32_t g) : index(i), generation(g) {}

        auto operator<=>(const IDHandle&) const = default;

        constexpr explicit operator bool() const noexcept { return index != 0; } // 0은 항상 invalid
        static constexpr IDHandle Invalid() noexcept { return IDHandle{ 0, 0 }; }
    };

    export template<typename ToTag, typename FromTag>
    constexpr IDHandle<ToTag> HandleCast(IDHandle<FromTag> h) noexcept
    {
        return IDHandle<ToTag>{ h.index, h.generation };
    }
}

// std::hash 특수화 (외부에서 IDHandle을 unordered_map 등의 키로 사용 가능하게 함)
namespace std
{
    template<typename Tag>
    struct hash<Core::IDHandle<Tag>>
    {
        std::size_t operator()(const Core::IDHandle<Tag>& h) noexcept
        {
            std::size_t seed = h.index;
            seed ^= (h.generation + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
            return seed;
        }
    };
}

/*
    사용법: Name에 이름 바꿔 넣기.
    import Core.Handle;
    struct NameTag {};
    using NameHandle = Core::IDHandle<NameTag>;
    inline constexpr NameHandle InvalidNameHandle = Core::IDHandle<NameTag>::Invalid();
*/