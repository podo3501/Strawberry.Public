export module Core.Bit:Align;

import std;
import Core.Assert;

namespace Core
{
    export template <typename T>
    concept Alignable = (std::integral<T> && !std::same_as<T, bool>) || std::is_pointer_v<T>;

    // 올림한 결과가 T에 담기지 않으면 Assert다 (음수는 0 쪽으로 올림한다)
    export template <Alignable T>
    [[nodiscard]] constexpr T AlignUp(T value, std::size_t alignment) noexcept
    {
        static_assert(sizeof(T) <= sizeof(std::uintptr_t), "값이 uintptr_t보다 크면 계산 중에 잘린다");
        Assert(std::has_single_bit(alignment));

        // 포인터든 정수든 uintptr_t로 통일해서 연산
        std::uintptr_t rawValue;
        if constexpr (std::is_pointer_v<T>)
            rawValue = reinterpret_cast<std::uintptr_t>(value);
        else
            rawValue = static_cast<std::uintptr_t>(value);

        std::uintptr_t aligned = (rawValue + alignment - 1) & ~(alignment - 1);

        // 다시 원래 타입 T로 복원해서 반환
        if constexpr (std::is_pointer_v<T>)
        {
            Assert(aligned >= rawValue);
            return reinterpret_cast<T>(aligned);
        }
        else
        {
            const T result = static_cast<T>(aligned);
            Assert(result >= value);
            return result;
        }
    }

    // AlignUp과 달리 alignment가 2의 거듭제곱이 아니어도 동작하는 범용 버전. 음수 value도 올림(0 쪽)한다.
    // 결과가 T에 담기는 한 중간 계산에서 넘치지 않고, 담기지 않으면 Assert다
    export template <typename T>
        requires std::integral<T> && (!std::same_as<T, bool>)
    [[nodiscard]] constexpr T AlignUpGeneric(T value, T alignment) noexcept
    {
        Assert(alignment > 0);

        if (std::has_single_bit(static_cast<std::size_t>(alignment)))
            return AlignUp(value, static_cast<std::size_t>(alignment));

        // 나머지의 부호는 value를 따른다. value + alignment - 1처럼 먼저 더하지 않는다
        const T remainder = static_cast<T>(value % alignment);
        if (remainder == 0)
            return value;

        if (remainder < 0)
            return static_cast<T>(value - remainder);   // 0 쪽으로 움직이므로 넘치지 않는다

        const T step = static_cast<T>(alignment - remainder);
        Assert(value <= std::numeric_limits<T>::max() - step);
        return static_cast<T>(value + step);
    }

    // 타입 크기가 N의 배수인지 컴파일 타임에 검사 (N이 2의 거듭제곱이 아니어도 동작)
    export template <typename T, std::size_t N>
        requires (N > 0)
    inline constexpr bool IsSizeAligned = (sizeof(T) % N == 0);
}