export module Client.Input:MouseState;

import std;
import :InputState;
import :MouseButton;
import Core.Geometry2D;

namespace Client
{
    export struct MouseState
    {
        Point prevPosition{};
        Point position{};

        int prevWheel{ 0 };
        int wheel{ 0 };

        static constexpr float WHEEL_STEP = 120.0f; // 윈도우 기본 단위

        constexpr Point GetDelta() const noexcept
        {
            return { position.x - prevPosition.x, position.y - prevPosition.y };
        }

        constexpr float GetWheelDelta() const noexcept
        {
            return static_cast<float>(wheel - prevWheel) / WHEEL_STEP;
        }

        std::array<InputState, static_cast<size_t>(MouseButton::Count)> buttons{};

        constexpr const InputState& operator[](MouseButton btn) const noexcept
        {
            return buttons[static_cast<size_t>(btn)];
        }

        constexpr InputState& operator[](MouseButton btn) noexcept
        {
            return buttons[static_cast<size_t>(btn)];
        }
    };
}