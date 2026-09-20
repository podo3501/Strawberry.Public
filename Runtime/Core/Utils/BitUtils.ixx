export module Core.Utils:Bit;

import std;

namespace Core
{
    // ---------------------------------------------------------
    // T가 ToNibbles()를 갖고 있는지 검사하는 concept
    // ---------------------------------------------------------
    export template <typename T>
        concept HasToNibbles = requires(const T & t)
    {
        t.ToNibbles();
    };

    // 인자 하나를 "니블 스칼라들의 tuple"로 펼침.
    // - ToNibbles()가 있으면 그 결과를 그대로 사용
    // - 없으면(순수 스칼라) tuple 하나짜리로 감쌈
    export template <typename T>
        constexpr auto FlattenToNibbles(const T& value)
    {
        if constexpr (HasToNibbles<T>)
        {
            return value.ToNibbles();
        }
        else
        {
            return std::make_tuple(value);
        }
    }

    // 4비트 고정 니블(nibble) 단위로 값을 순서대로 패킹. 최대 8개까지 (4 * 8 = 32비트)
    // 스칼라값뿐 아니라, ToNibbles()를 제공하는 구조체도
    // 인자로 그대로 넘길 수 있음 -> 내부적으로 스칼라들로 펼쳐진 뒤 패킹됨.
    export template <typename... Ts>
        constexpr std::uint32_t PackNibbles(const Ts&... values)
    {
        return std::apply([]<typename... Nibbles>(Nibbles... nibbles) -> std::uint32_t
        {
            static_assert(sizeof...(nibbles) <= 8, "니블 8개(32비트)를 초과할 수 없습니다.");
            assert(((static_cast<std::uint32_t>(nibbles) < 16) && ...)); // 값이 16을 넘어가면 짤린다.

            std::uint32_t packed = 0;
            std::uint32_t offset = 0;
            ((packed |= (static_cast<std::uint32_t>(nibbles) & 0x0F) << offset, offset += 4), ...);
            return packed;
        }, std::tuple_cat(FlattenToNibbles(values)...));
    }

    export template <typename... Ts>
        constexpr std::tuple<Ts...> UnpackNibbles(std::uint32_t packed, std::uint32_t startIndex = 0)
    {
        static_assert(sizeof...(Ts) <= 8, "니블 8개(32비트)를 초과할 수 없습니다.");
        std::uint32_t offset = startIndex * 4;
        return std::tuple<Ts...>{
            [&]() -> Ts
                {
                    Ts value = static_cast<Ts>((packed >> offset) & 0x0F);
                    offset += 4;
                    return value;
                }()...
        };
    }
}