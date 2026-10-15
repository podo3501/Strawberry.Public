export module Service.Input:KeyboardState;

import std;
import :ButtonState;
import Contract.Input.Types;

export struct KeyboardState
{
    std::array<ButtonState, static_cast<size_t>(KeyCode::Count)> keys{};

    [[nodiscard]] const ButtonState& operator[](KeyCode key) const noexcept
    {
        return keys[static_cast<size_t>(key)];
    }

    [[nodiscard]] ButtonState& operator[](KeyCode key) noexcept
    {
        return keys[static_cast<size_t>(key)];
    }
};
