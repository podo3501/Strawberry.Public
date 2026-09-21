export module Core.Bit:Align;

import std;
import Core.Assert;

namespace Core
{
    export template <typename T>
        concept Alignable = (std::integral<T> && !std::same_as<T, bool>) || std::is_pointer_v<T>;

    export template <Alignable T>
        [[nodiscard]] constexpr T AlignUp(T value, std::size_t alignment) noexcept
    {
        Assert(std::has_single_bit(alignment));

        // 포인터든 정수든 uintptr_t로 통일해서 연산
        std::uintptr_t rawValue;
        if constexpr (std::is_pointer_v<T>)
            rawValue = reinterpret_cast<std::uintptr_t>(value);
        else
            rawValue = static_cast<std::uintptr_t>(value);

        std::uintptr_t aligned = (rawValue + alignment - 1) & ~(alignment - 1);

        // 다시 원래 타입 T로 안전하게 복원해서 반환
        if constexpr (std::is_pointer_v<T>)
            return reinterpret_cast<T>(aligned);
        else
            return static_cast<T>(aligned);
    }

    export template <Alignable T>
        [[nodiscard]] constexpr T AlignDown(T value, std::size_t alignment) noexcept
    {
        Assert(std::has_single_bit(alignment));

        std::uintptr_t rawValue;
        if constexpr (std::is_pointer_v<T>)
            rawValue = reinterpret_cast<std::uintptr_t>(value);
        else
            rawValue = static_cast<std::uintptr_t>(value);

        std::uintptr_t aligned = rawValue & ~(alignment - 1);

        if constexpr (std::is_pointer_v<T>)
            return reinterpret_cast<T>(aligned);
        else
            return static_cast<T>(aligned);
    }

    // AlignUp과 달리 alignment가 2의 거듭제곱이 아니어도 동작하는 범용 버전
    export template <typename T>
        requires std::integral<T> && (!std::same_as<T, bool>)
    [[nodiscard]] constexpr T AlignUpGeneric(T value, T alignment) noexcept
    {
        Assert(alignment > 0);

        if (std::has_single_bit(static_cast<std::size_t>(alignment)))
            return AlignUp(value, static_cast<std::size_t>(alignment));

        return ((value + alignment - 1) / alignment) * alignment;
    }
}