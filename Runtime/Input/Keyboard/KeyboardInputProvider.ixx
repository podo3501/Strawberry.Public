module;

#ifdef _DEBUG
#pragma comment(lib, "DirectXTK12_Debug.lib")
#else
#pragma comment(lib, "DirectXTK12_Release.lib")
#endif

#include <Windows.h>
#include "DirectXTK12/Keyboard.h"

export module Runtime.Input:KeyboardInputProvider;

import std;
import Client.Input.Contract;
import :KeyMap;

class KeyboardInputProvider : public IKeyboardInputProvider
{
public:
	KeyboardInputProvider() noexcept = default;
	~KeyboardInputProvider() override = default;

	void Update() noexcept override
	{
		const auto& dxState = m_keyboard.GetState();

		for (auto [dxKey, code] : KeyMap)
		{
			InputState& state = m_keyboardState[code];
			state.Update(dxState.IsKeyDown(dxKey));
		}
	}

	void ProcessMessage(std::uint32_t msg, std::uint64_t wParam, std::int64_t lParam) noexcept override
	{
		DirectX::Keyboard::ProcessMessage(msg, static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam));
	}

	const KeyboardState& GetState() const noexcept override
	{
		return m_keyboardState;
	}

private:
	static inline DirectX::Keyboard m_keyboard{}; // inline static을 사용하면 .cpp 파일에 별도로 정적 변수 정의를 작성할 필요가 없음.
	KeyboardState m_keyboardState{};
};

std::unique_ptr<IKeyboardInputProvider> CreateDXKeyboardInputProvider()
{
	return std::make_unique<KeyboardInputProvider>();
}