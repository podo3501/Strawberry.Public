export module Client.Input:Service;

import std;
import :KeyCode;
import :KeyboardState;
import :MouseState;
import :IKeyboardInputProvider;
import :IMouseInputProvider;
import Core.Math;

namespace Client
{
    export class InputService
    {
    public:
        ~InputService();
        InputService() = delete;

        static std::unique_ptr<InputService> Create(
            std::unique_ptr<IKeyboardInputProvider> keyboardProvider,
            std::unique_ptr<IMouseInputProvider> mouseProvider);

        bool IsKeyPressed(KeyCode key) const noexcept;
        bool IsKeyHeld(KeyCode key) const noexcept;
        bool IsKeyReleased(KeyCode key) const noexcept;
        bool IsKeyUp(KeyCode key) const noexcept;

        bool IsMouseButtonPressed(MouseButton button) const noexcept;
        bool IsMouseButtonHeld(MouseButton button) const noexcept;
        bool IsMouseButtonReleased(MouseButton button) const noexcept;
        bool IsMouseButtonUp(MouseButton button) const noexcept;

        bool AreKeysCombo(std::initializer_list<KeyCode> heldKeys, KeyCode pressedKey) const noexcept;
        bool AreKeysMouseCombo(std::initializer_list<KeyCode> heldKeys, MouseButton pressedButton) const noexcept;

        void Update() noexcept;

        void SetMousePositionOffset(const Core::Point& offset) noexcept;
        KeyboardState GetKeyboardState() const noexcept;
        MouseState GetMouseState() const noexcept;

    private:
        InputService(
            std::unique_ptr<IKeyboardInputProvider> keyboardProvider,
            std::unique_ptr<IMouseInputProvider> mouseProvider) noexcept;

        std::unique_ptr<IKeyboardInputProvider> m_keyboardProvider;
        std::unique_ptr<IMouseInputProvider> m_mouseProvider;

        KeyboardState m_keyboardState{};
        MouseState m_mouseState{};

        Core::Point m_mouseOffset{};
    };
}