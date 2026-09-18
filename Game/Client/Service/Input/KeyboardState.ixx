export module KeyboardState;

import std;
import KeyCode;
import InputState;

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