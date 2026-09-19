module;

#include "DirectXTK12/Keyboard.h"

export module Input.DirectX:KeyboardInputProvider;

import std;
import Client.Input;
import :KeyMap;

export class KeyboardInputProvider : public Client::IKeyboardInputProvider
{
public:
	KeyboardInputProvider() noexcept = default;
	~KeyboardInputProvider() override = default;

	void Update() noexcept override
	{
		const auto& dxState = m_keyboard.GetState();

		for (auto [dxKey, code] : KeyMap)
		{
			Client::InputState& state = m_keyboardState[code];
			state.Update(dxState.IsKeyDown(dxKey));
		}
	}

	const Client::KeyboardState& GetState() const noexcept override
	{
		return m_keyboardState;
	}

private:
	static inline DirectX::Keyboard m_keyboard{}; // inline static을 사용하면 .cpp 파일에 별도로 정적 변수 정의를 작성할 필요가 없음.
	Client::KeyboardState m_keyboardState{};
};

export std::unique_ptr<Client::IKeyboardInputProvider> CreateDXKeyboardInputProvider()
{
	return std::make_unique<KeyboardInputProvider>();
}