module;

#ifdef _DEBUG
#pragma comment(lib, "DirectXTK12_Debug.lib")
#else
#pragma comment(lib, "DirectXTK12_Release.lib")
#endif
#pragma comment(lib, "Comctl32.lib")

#include <Windows.h>
#include "DirectXTK12/Keyboard.h"
#include "DirectXTK12/Mouse.h"

export module DxInputBackend;

import std;
import Contract.Input.Types;
import Contract.Input.Interfaces;
import Core.Utils;

export class InputBackend final : public IInputBackend
{
public:
    explicit InputBackend(HWND hwnd);
    ~InputBackend() override;

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

    void Detach() noexcept;

    static inline DirectX::Keyboard m_keyboard{};
    static inline DirectX::Mouse m_mouse{};

    HWND m_hwnd{ nullptr };
    std::array<bool, Core::EnumSize<KeyCode>> m_keyDown{};
    std::array<bool, Core::EnumSize<MouseButton>> m_buttonDown{};
    int m_prevWheel{ 0 };
};