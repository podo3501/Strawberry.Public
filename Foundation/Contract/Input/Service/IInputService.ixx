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
	virtual std::string_view GetTextInput() const noexcept = 0; // 이번 Update에서 입력된 문자(UTF-8). 다음 Update에서 교체된다.

	virtual bool AreKeysCombo(std::initializer_list<KeyCode> heldKeys, KeyCode key) const noexcept = 0;
	virtual bool AreKeysCombo(std::initializer_list<KeyCode> heldKeys, MouseButton button) const noexcept = 0;

	virtual void Update(float dt) noexcept = 0; // dt: 지난 프레임부터 흐른 시간(초). 키 반복 시점(ButtonState::IsRepeated)을 계산하는 데 쓴다
};

export std::unique_ptr<IInputService> CreateInputService(std::unique_ptr<IInputBackend> backend);