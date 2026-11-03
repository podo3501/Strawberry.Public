module;

#ifdef _DEBUG
#pragma comment(lib, "DirectXTK12_Debug.lib")
#else
#pragma comment(lib, "DirectXTK12_Release.lib")
#endif
#pragma comment(lib, "Comctl32.lib")

#include <Windows.h>
#include <CommCtrl.h>
#include "DirectXTK12/Keyboard.h"
#include "DirectXTK12/Mouse.h"

module DxInput;

import :KeyMap;
import :MouseButtonMap;
import Core.Math;

InputBackend::InputBackend(HWND hwnd) : m_hwnd(hwnd)
{
    ::SetWindowSubclass(m_hwnd, &SubclassProc, SubclassId, reinterpret_cast<DWORD_PTR>(this));
    m_mouse.SetWindow(m_hwnd);
}

InputBackend::~InputBackend()
{
    Detach();
}

InputFrame InputBackend::TakeFrame() noexcept
{
    InputFrame frame{};

    TakeKeyboard(frame);
    TakeMouse(frame);
    frame.text = std::exchange(m_text, {});

    return frame;
}

void InputBackend::OnChar(wchar_t unit) noexcept
{
    // Backspace/Enter/Tab/Esc 같은 제어 문자는 KeyCode로 전달되므로 문자에서 제외한다.
    if (unit < 0x20 || unit == 0x7F)
        return;

    char32_t codePoint = unit;

    if (IS_HIGH_SURROGATE(unit))
    {
        m_highSurrogate = unit; // 뒤따라오는 low surrogate와 합친다.
        return;
    }

    if (IS_LOW_SURROGATE(unit))
    {
        if (!m_highSurrogate)
            return; // 짝 없는 low surrogate는 버린다.

        codePoint = 0x10000 + ((static_cast<char32_t>(m_highSurrogate) - 0xD800) << 10) + (unit - 0xDC00);
    }

    m_highSurrogate = 0;
    Core::UTF32ToUTF8Char(codePoint, m_text);
}

void InputBackend::TakeKeyboard(InputFrame& frame) noexcept
{
    const auto dxState = m_keyboard.GetState();

    for (const auto& [dxKey, code] : KeyMap)
    {
        const bool isDown = dxState.IsKeyDown(dxKey);
        bool& wasDown = m_keyDown[static_cast<std::size_t>(code)];

        if (isDown != wasDown)
        {
            wasDown = isDown;
            frame.keyEvents.push_back({ code, isDown });
        }
    }
}

void InputBackend::TakeMouse(InputFrame& frame) noexcept
{
    const auto ms = m_mouse.GetState();

    for (const auto& [button, member] : MouseButtonMap)
    {
        const bool isDown = ms.*member;
        bool& wasDown = m_buttonDown[static_cast<std::size_t>(button)];

        if (isDown != wasDown)
        {
            wasDown = isDown;
            frame.mouseButtonEvents.push_back({ button, isDown });
        }
    }

    frame.position = Core::Point{ ms.x, ms.y };
    frame.wheelDelta = ms.scrollWheelValue - m_prevWheel;
    m_prevWheel = ms.scrollWheelValue;
}

LRESULT CALLBACK InputBackend::SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
    UINT_PTR /*id*/, DWORD_PTR refData)
{
    auto* self = reinterpret_cast<InputBackend*>(refData);

    // 상태 갱신은 DirectXTK가 한다 (포커스 상실 시 상태 리셋도 내부에서 처리)
    DirectX::Keyboard::ProcessMessage(msg, wParam, lParam);
    DirectX::Mouse::ProcessMessage(msg, wParam, lParam);

    if (msg == WM_CHAR && self)
        self->OnChar(static_cast<wchar_t>(wParam));

    if (msg == WM_NCDESTROY && self)
        self->Detach(); // 윈도우가 먼저 파괴되는 경우: 스스로 해제하고 체인을 이어준다.

    return ::DefSubclassProc(hwnd, msg, wParam, lParam);
}

void InputBackend::Detach() noexcept
{
    if (m_hwnd)
    {
        ::RemoveWindowSubclass(m_hwnd, &SubclassProc, SubclassId);
        m_hwnd = nullptr;
    }
}

std::unique_ptr<IInputBackend> CreateDxInputBackend(NativeWindowHandle handle)
{
    return std::make_unique<InputBackend>(static_cast<HWND>(handle));
}