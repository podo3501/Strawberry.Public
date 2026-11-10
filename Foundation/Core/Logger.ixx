module;

#include <spdlog/fwd.h>

#ifdef _DEBUG
#pragma comment(lib, "spdlog_Debug.lib")
#else
#pragma comment(lib, "spdlog_Release.lib")
#endif

export module Core.Logger;

import std;

export namespace Core
{
    class Logger
    {
    public:
        // logFile에 기록하고 toConsole이면 콘솔에도 출력한다. 이미 초기화했으면 무시한다
        static void Initialize(std::string_view logFile = "Engine.log", bool toConsole = true);
        static void Shutdown();

        template<typename... Args>
        static void Trace(
            std::format_string<Args...> fmt,
            Args&&... args)
        {
            Write(
                Level::Trace,
                std::format(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        static void Info(
            std::format_string<Args...> fmt,
            Args&&... args)
        {
            Write(
                Level::Info,
                std::format(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        static void Warn(
            std::format_string<Args...> fmt,
            Args&&... args)
        {
            Write(
                Level::Warn,
                std::format(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        static void Error(
            std::format_string<Args...> fmt,
            Args&&... args)
        {
            Write(
                Level::Error,
                std::format(fmt, std::forward<Args>(args)...));
        }

    private:
        enum class Level
        {
            Trace,
            Info,
            Warn,
            Error
        };

        static void Write(
            Level level,
            std::string_view message);

        static std::shared_ptr<spdlog::logger> s_logger;
    };
}