module;

#include <Windows.h>

export module App.GameWindow;

import std;
import Core.Math;

// 게임 창. Win32 창을 만들고 메시지를 처리한다 (구현: GameWindow.cpp)
// 입력 백엔드가 이 창을 서브클래싱하므로 입력 서비스보다 먼저 만들고 나중에 해제해야 한다
export class GameWindow
{
public:
    GameWindow(std::wstring_view title, const Core::Size& clientSize);
    ~GameWindow();
    GameWindow(const GameWindow&) = delete;
    GameWindow& operator=(const GameWindow&) = delete;

    bool IsValid() const noexcept { return m_hwnd != nullptr; }
    HWND Handle() const noexcept { return m_hwnd; }
    Core::Size ClientSize() const;   // 최소화되면 0이 된다

    // 쌓인 메시지를 모두 처리한다. 창이 닫혀 종료해야 하면 false
    bool PumpMessages();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HINSTANCE m_instance{};
    HWND m_hwnd{};
};
