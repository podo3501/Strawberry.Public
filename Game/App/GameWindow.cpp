module;

#include <Windows.h>

module App.GameWindow;

import std;

namespace
{
    constexpr const wchar_t* kClassName = L"StrawberryGameWindow";
}

GameWindow::GameWindow(std::wstring_view title, const Core::Size& clientSize)
    : m_instance{ GetModuleHandleW(nullptr) }
{
    WNDCLASSEXW wc{ sizeof(wc) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = m_instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);

    // 요청한 크기가 테두리를 뺀 클라이언트 영역이 되도록 창 크기를 늘린다
    RECT rect{ 0, 0, static_cast<LONG>(clientSize.width), static_cast<LONG>(clientSize.height) };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    const std::wstring titleText{ title };
    m_hwnd = CreateWindowExW(0, kClassName, titleText.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, m_instance, nullptr);

    if (m_hwnd)
    {
        ShowWindow(m_hwnd, SW_SHOWDEFAULT);
        UpdateWindow(m_hwnd);
    }
}

GameWindow::~GameWindow()
{
    if (m_hwnd) DestroyWindow(m_hwnd);
    UnregisterClassW(kClassName, m_instance);
}

Core::Size GameWindow::ClientSize() const
{
    RECT rect{};
    if (!m_hwnd || !GetClientRect(m_hwnd, &rect)) return {};
    return { static_cast<std::uint32_t>(rect.right - rect.left), static_cast<std::uint32_t>(rect.bottom - rect.top) };
}

bool GameWindow::PumpMessages()
{
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        if (msg.message == WM_QUIT) return false;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return true;
}

LRESULT CALLBACK GameWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;   // Alt 키로 메뉴가 열리지 않게 한다
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
