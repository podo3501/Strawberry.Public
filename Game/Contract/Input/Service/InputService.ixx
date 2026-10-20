export module Contract.Input.Service:IInputService;

import std;
import :ButtonState;
import :MouseState;
import Contract.Input.Shared;
import Contract.Input.Backend;
import Core.Math;

export struct IInputService
{
	virtual ~IInputService() = default;

	virtual const ButtonState& GetKey(KeyCode key) const noexcept = 0;
	virtual const ButtonState& GetMouseButton(MouseButton button) const noexcept = 0;

	virtual Core::Point GetMousePosition() const noexcept = 0;
	virtual Core::Point GetMouseDelta() const noexcept = 0;
	virtual float GetMouseWheelDelta() const noexcept = 0;
	virtual const MouseState& GetMouseState() const noexcept = 0;

	virtual bool AreKeysCombo(std::initializer_list<KeyCode> heldKeys, KeyCode key) const noexcept = 0;
	virtual bool AreKeysCombo(std::initializer_list<KeyCode> heldKeys, MouseButton button) const noexcept = 0;

	virtual void Update() noexcept = 0;
};

export std::unique_ptr<IInputService> CreateInputService(std::unique_ptr<IInputBackend> backend);