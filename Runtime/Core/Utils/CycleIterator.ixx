export module Core.CycleIterator;

import std;
import Core.Assert;

namespace Core
{
    export class CycleIterator
    {
    public:
        constexpr CycleIterator() noexcept = default;

        constexpr CycleIterator(int s, int e) noexcept
        {
            SetRange(s, e);
        }

        constexpr void SetRange(int s, int e) noexcept
        {
            Assert(s < e);

            start = s;
            end = e;
            current = s;
        }

        constexpr int GetCurrent() const noexcept
        {
            return current;
        }

        constexpr int Increase() noexcept
        {
            Assert(start < end);

            current = start + Mod(current + 1 - start, end - start);
            return current;
        }

        constexpr int Decrease() noexcept
        {
            Assert(start < end);

            current = start + Mod(current - 1 - start, end - start);
            return current;
        }

        constexpr void Reset() noexcept
        {
            current = start;
        }

    private:
        static constexpr int Mod(int value, int range) noexcept
        {
            return (value % range + range) % range;
        }

    private:
        int current{ 0 };
        int start{ 0 };
        int end{ 0 };
    };
}