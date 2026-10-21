export module Service.Input;

import std;
import :KeyboardState;
import Contract.Input.Shared;
import Contract.Input.Service;
import Contract.Input.Backend;
import Core.Math;

class InputService : public IInputService
{
public:
	virtual ~InputService() override = default;
	explicit InputService(std::unique_ptr<IInputBackend> backend) noexcept;

	const ButtonState& GetKey(KeyCode key) const noexcept { return m_keyboardState[key]; }
	const ButtonState& GetMouseButton(MouseButton button) const noexcept { return m_mouseState[button]; }

	bool AreKeysCombo(std::initializer_list<KeyCode> heldKeys, KeyCode key) const noexcept;
	bool AreKeysCombo(std::initializer_list<KeyCode> heldKeys, MouseButton button) const noexcept;

	void Update() noexcept;

	Core::Point GetMousePosition() const noexcept { return m_mouseState.position; }
	Core::Point GetMouseDelta() const noexcept { return m_mouseState.GetDelta(); }
	float GetMouseWheelDelta() const noexcept { return m_mouseState.GetWheelDelta(); }
	const MouseState& GetMouseState() const noexcept { return m_mouseState; }

private:
	bool AreKeysDown(std::initializer_list<KeyCode> keys) const noexcept;

	std::unique_ptr<IInputBackend> m_backend;
	KeyboardState m_keyboardState{};
	MouseState m_mouseState{};
	bool m_mouseInitialized{ false };
};