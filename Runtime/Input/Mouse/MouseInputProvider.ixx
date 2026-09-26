module;

#include <Windows.h>
#include "DirectXTK12/Mouse.h"

export module Runtime.Input:MouseInputProvider;

import std;
import Core.Assert;
import Client.Input.Contract;

class MouseInputProvider : public IMouseInputProvider
{
public:
	MouseInputProvider() noexcept = default;
	explicit MouseInputProvider(HWND hwnd) noexcept
	{
		m_mouse.SetWindow(hwnd);
	}

	~MouseInputProvider() override = default;

	void Update() noexcept override
	{
		const auto raw = m_mouse.GetState();

		if (!m_initialized) // 처음에 마우스 튀는 현상 방지
		{
			UpdateButton(raw, UpdateMode::Sync);

			const auto rawPos = Core::ToPoint(raw.x, raw.y);
			m_state.prevPosition = rawPos;
			m_state.position = rawPos;

			m_state.prevWheel = raw.scrollWheelValue;
			m_state.wheel = raw.scrollWheelValue;

			m_initialized = true;
			return;
		}

		UpdateButton(raw, UpdateMode::Normal);

		m_state.prevPosition = m_state.position;
		m_state.position = Core::ToPoint(raw.x, raw.y);

		m_state.prevWheel = m_state.wheel;
		m_state.wheel = raw.scrollWheelValue;
	}

	const MouseState& GetState() const noexcept override
	{
		Core::Assert(m_initialized); // MouseInputProvider not initialized
		return m_state;
	}

private:
	enum class UpdateMode { Sync, Normal };

	void UpdateButton(const DirectX::Mouse::State& dxState, UpdateMode mode)
	{
		for (size_t i = 0; i < ButtonCount; ++i)
		{
			const bool value = dxState.*ButtonMap[i];

			if (mode == UpdateMode::Sync)
			{
				m_state.buttons[i].current = value;
				m_state.buttons[i].previous = value;
				continue;
			}

			m_state.buttons[i].Update(value);
		}
	}

private:
	// 멤버 가리키기 포인터(Member Pointer) 맵
	static inline constexpr bool DirectX::Mouse::State::* ButtonMap[] =
	{
		&DirectX::Mouse::State::leftButton,
		&DirectX::Mouse::State::rightButton,
		&DirectX::Mouse::State::middleButton,
		&DirectX::Mouse::State::xButton1,
		&DirectX::Mouse::State::xButton2
	};

	static inline constexpr size_t ButtonCount = std::size(ButtonMap);
	static_assert(ButtonCount == static_cast<size_t>(MouseButton::Count),
		"ButtonMap and MouseState::buttons size mismatch");

	static inline DirectX::Mouse m_mouse{}; // inline static으로 선언하여 별도 .cpp 정적 변수 정의를 제거
	bool m_initialized{ false };
	MouseState m_state{};
};

std::unique_ptr<IMouseInputProvider> CreateDXMouseInputProvider(NativeWindowHandle handle)
{
	return std::make_unique<MouseInputProvider>(reinterpret_cast<HWND>(handle));
}