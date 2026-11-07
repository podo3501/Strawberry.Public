#include <Windows.h>
#include <objbase.h>

import std;
import App.Application;
import Core.Logger;

namespace
{
    // COM 초기화와 정리. Application보다 먼저 만들어서 서비스가 모두 해제된 뒤에 CoUninitialize가 불리게 한다
    class ComScope
    {
    public:
        ComScope() : m_initialized(SUCCEEDED(CoInitialize(nullptr))) {}
        ~ComScope() { if (m_initialized) CoUninitialize(); }
        ComScope(const ComScope&) = delete;
        ComScope& operator=(const ComScope&) = delete;

    private:
        bool m_initialized;
    };
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    Core::Logger::Initialize();
    ComScope com;

    Application app;
    if (!app.Initialize())
        return 1;

    return app.Run();
}
