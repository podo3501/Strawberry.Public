module;

#include "DirectXTK12/Keyboard.h"

export module DXKeyboardInputProvider;

import std;
import IKeyboardInputProvider;
import KeyboardState;
import InputState;
import DXKeyMap; // DXKeyMap도 모듈화되어 있다고 가정 (헤더라면 module; 구역에 #include)

export class DXKeyboardInputProvider : public IKeyboardInputProvider
{
public:
	DXKeyboardInputProvider() noexcept = default;
	~DXKeyboardInputProvider() override = default;

	void Update() noexcept override
	{
		const auto& dxState = m_keyboard.GetState();

		for (auto [dxKey, code] : DXKeyMap)
		{
			InputState& state = m_keyboardState[code];
			state.Update(dxState.IsKeyDown(dxKey));
		}
	}

	const KeyboardState& GetState() const noexcept override
	{
		return m_keyboardState;
	}

private:
	// inline static을 사용하면 .cpp 파일에 별도로 정적 변수 정의를 작성할 필요가 없습니다.
	static inline DirectX::Keyboard m_keyboard{};
	KeyboardState m_keyboardState{};
};

export std::unique_ptr<IKeyboardInputProvider> CreateDXKeyboardInputProvider()
{
	return std::make_unique<DXKeyboardInputProvider>();
}