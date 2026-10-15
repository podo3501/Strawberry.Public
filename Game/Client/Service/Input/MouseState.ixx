export module Service.Input:MouseState;

import std;
import :ButtonState;
import Core.Math;
import Contract.Input.Types;

export struct MouseState
{
    Core::Point prevPosition{};
    Core::Point position{};

    int prevWheel{ 0 };
    int wheel{ 0 };

    static constexpr float WHEEL_STEP = 120.0f; // 윈도우 기본 단위

    constexpr Core::Point GetDelta() const noexcept
    {
        return { position.x - prevPosition.x, position.y - prevPosition.y };
    }

    constexpr float GetWheelDelta() const noexcept
    {
        return static_cast<float>(wheel - prevWheel) / WHEEL_STEP;
    }

    std::array<ButtonState, static_cast<size_t>(MouseButton::Count)> buttons{};

    constexpr const ButtonState& operator[](MouseButton btn) const noexcept
    {
        return buttons[static_cast<size_t>(btn)];
    }

    constexpr ButtonState& operator[](MouseButton btn) noexcept
    {
        return buttons[static_cast<size_t>(btn)];
    }
};