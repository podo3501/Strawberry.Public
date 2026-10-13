export module Service.InputService;

import std;
import :KeyboardState;
import :MouseState;
import Contract.Input.Interfaces;
import Contract.Input.Types;
import Core.Math;

export class InputService
{
public:
    ~InputService();
    InputService() = delete;

	static std::unique_ptr<InputService> Create(std::unique_ptr<IInputBackend> backend);

    bool IsKeyPressed(KeyCode key) const noexcept { return m_keyboardState[key].IsPressed(); }
    bool IsKeyHeld(KeyCode key) const noexcept { return m_keyboardState[key].IsHeld(); }
    bool IsKeyReleased(KeyCode key) const noexcept { return m_keyboardState[key].IsReleased(); }
    bool IsKeyUp(KeyCode key) const noexcept { return m_keyboardState[key].IsUp(); }

    bool IsMouseButtonPressed(MouseButton button) const noexcept { return m_mouseState[button].IsPressed(); }
    bool IsMouseButtonHeld(MouseButton button) const noexcept { return m_mouseState[button].IsHeld(); }
    bool IsMouseButtonReleased(MouseButton button) const noexcept { return m_mouseState[button].IsReleased(); }
    bool IsMouseButtonUp(MouseButton button) const noexcept { return m_mouseState[button].IsUp(); }

    bool AreKeysCombo(std::initializer_list<KeyCode> heldKeys, KeyCode pressedKey) const noexcept;
    bool AreKeysCombo(std::initializer_list<KeyCode> heldKeys, MouseButton pressedButton) const noexcept;

    void Update() noexcept;

	Core::Point GetMousePosition() const noexcept { return m_mouseState.position; }
    Core::Point GetMouseDelta() const noexcept { return m_mouseState.GetDelta(); }
    float GetMouseWheelDelta() const noexcept { return m_mouseState.GetWheelDelta(); }
    const MouseState& GetMouseState() const noexcept { return m_mouseState; }

private:
    InputService(std::unique_ptr<IInputBackend> backend) noexcept;
    bool AreKeysDown(std::initializer_list<KeyCode> keys) const noexcept;

    std::unique_ptr<IInputBackend> m_backend;
    KeyboardState m_keyboardState{};
    MouseState m_mouseState{};
    bool m_mouseInitialized{ false };
};