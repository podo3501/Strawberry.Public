module;

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

module Core.Logger;

import std;

namespace Core
{
    std::shared_ptr<spdlog::logger> Logger::s_logger;
    bool Logger::s_errorToStderr{ false };

    bool Logger::Initialize(std::string_view logFile, bool toConsole)
    {
        if (s_logger)
            return true;

        // 싱크를 만들다가 예외가 나지 않게 폴더를 만들고 파일을 열 수 있는지 먼저 확인한다
        const std::filesystem::path path{ std::u8string(reinterpret_cast<const char8_t*>(logFile.data()), logFile.size()) };
        std::error_code error;
        if (path.has_parent_path())
            std::filesystem::create_directories(path.parent_path(), error);

        if (error || !std::ofstream{ path, std::ios::trunc })
        {
            s_errorToStderr = toConsole;
            if (toConsole)
                std::println(std::cerr, "Logger: 로그 파일을 열 수 없음 ({})", logFile);
            return false;
        }

        s_errorToStderr = false;
        std::vector<spdlog::sink_ptr> sinks;

        if (toConsole)
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            std::string{ logFile },
            true)); // true면 새로 log 파일 만들기. false면 log 파일 뒤에 이어쓰기.

        s_logger = std::make_shared<spdlog::logger>(
            "Engine",
            sinks.begin(),
            sinks.end());

        s_logger->set_level(spdlog::level::trace);
        s_logger->flush_on(spdlog::level::err);   // 오류는 바로 파일에 남긴다 (프로세스가 갑자기 끝나도 읽을 수 있게)

        s_logger->set_pattern(
            "[%H:%M:%S.%e] [%^%l%$] %v");

        spdlog::set_default_logger(s_logger);
        return true;
    }

    void Logger::Shutdown()
    {
        s_logger.reset();
        s_errorToStderr = false;
        spdlog::shutdown();
    }

    void Logger::Write(
        Level level,
        std::string_view message)
    {
        if (!s_logger)
        {
            if (s_errorToStderr && level == Level::Error)
                std::println(std::cerr, "[error] {}", message);
            return;
        }

        switch (level)
        {
        case Level::Trace:
            s_logger->trace(message);
            break;

        case Level::Info:
            s_logger->info(message);
            break;

        case Level::Warn:
            s_logger->warn(message);
            break;

        case Level::Error:
            s_logger->error(message);
            break;
        }
    }
}