#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

namespace servercore
{
    class Logger
    {
    public:
        static void Initialize(const std::string& loggerName, spdlog::level::level_enum leven = spdlog::level::trace, 
                                size_t queueSize = 8192, spdlog::level::level_enum flushLevel = spdlog::level::err);
        static void Shutdown();
        static std::shared_ptr<spdlog::logger>& GetInstance();

        template<typename... Args>
        static void Log(const spdlog::source_loc& loc, spdlog::level::level_enum level, const std::string& threadName, fmt::format_string<Args...> fmtStr, Args&&... args)
        {
            auto logger = GetInstance();
            if(logger == nullptr)
                return;

            logger->log(loc, level, "[{}] {}", threadName, fmt::format(fmtStr, std::forward<Args>(args)...));
        }

    private:
        static void       SetPattern(const std::shared_ptr<spdlog::logger>& logger);
    };
}

#define NC_LOG_TRACE(...)    servercore::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::trace,     servercore::LThreadName , __VA_ARGS__)
#define NC_LOG_DEBUG(...)    servercore::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::debug,     servercore::LThreadName , __VA_ARGS__)
#define NC_LOG_INFO(...)     servercore::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::info,      servercore::LThreadName , __VA_ARGS__)
#define NC_LOG_WARN(...)     servercore::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::warn,      servercore::LThreadName , __VA_ARGS__)
#define NC_LOG_ERROR(...)    servercore::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::err,       servercore::LThreadName , __VA_ARGS__)
#define NC_LOG_CRITICAL(...) servercore::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::critical,  servercore::LThreadName , __VA_ARGS__)