export module Contract.Input:KeyboardState;

import std;
import :InputState;
import Contract.Input.Types;

export struct KeyboardState
{
    std::array<InputState, static_cast<size_t>(KeyCode::Count)> keys{};

    [[nodiscard]] const InputState& operator[](KeyCode key) const noexcept
    {
        return keys[static_cast<size_t>(key)];
    }

    [[nodiscard]] InputState& operator[](KeyCode key) noexcept
    {
        return keys[static_cast<size_t>(key)];
    }
};
