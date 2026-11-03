module;

#include <Windows.h>
#include "DirectXTK12/Keyboard.h"
#include "DirectXTK12/Mouse.h"

export module DxInput;

import std;
import Contract.Input.Shared;
import Contract.Input.Backend;
import Core.Utils;

class InputBackend final : public IInputBackend
{
public:
    ~InputBackend() override;
    explicit InputBackend(HWND hwnd);

    InputBackend(const InputBackend&) = delete;
    InputBackend& operator=(const InputBackend&) = delete;

    InputFrame TakeFrame() noexcept override;

private:
    static constexpr UINT_PTR SubclassId = 1;

    void TakeKeyboard(InputFrame& frame) noexcept;
    void TakeMouse(InputFrame& frame) noexcept;

    static LRESULT CALLBACK SubclassProc(
        HWND hwnd, 
        UINT msg, 
        WPARAM wParam, 
        LPARAM lParam,
        UINT_PTR id, DWORD_PTR refData);

    void OnChar(wchar_t unit) noexcept;
    void Detach() noexcept;

    static inline DirectX::Keyboard m_keyboard{};
    static inline DirectX::Mouse m_mouse{};

    HWND m_hwnd{ nullptr };
    std::array<bool, Core::EnumSize<KeyCode>> m_keyDown{};
    std::array<bool, Core::EnumSize<MouseButton>> m_buttonDown{};
    int m_prevWheel{ 0 };
    std::string m_text;             // 프레임 사이에 도착한 문자(UTF-8). TakeFrame에서 비운다.
    wchar_t m_highSurrogate{ 0 };   // 서로게이트 쌍의 앞 절반을 기다리는 중
};