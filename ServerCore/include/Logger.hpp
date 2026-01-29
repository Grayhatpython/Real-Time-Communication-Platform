#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

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
        static void Log(const spdlog::source_loc& loc, spdlog::level::level_enum level, fmt::format_string<Args...> fmtStr, Args&&... args)
        {
            auto logger = GetInstance();
            if(logger == nullptr)
                return;

            logger->log(loc, level, fmtStr, std::forward<Args>(args)...);
        }

    private:
        static void                             SetPattern(const std::shared_ptr<spdlog::logger>& logger);
    };
}

#define NC_LOG_TRACE(...)    servercore::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::trace,    __VA_ARGS__)
#define NC_LOG_DEBUG(...)    servercore::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::debug,    __VA_ARGS__)
#define NC_LOG_INFO(...)     servercore::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::info,     __VA_ARGS__)
#define NC_LOG_WARN(...)     servercore::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::warn,     __VA_ARGS__)
#define NC_LOG_ERROR(...)    servercore::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::err,      __VA_ARGS__)
#define NC_LOG_CRITICAL(...) servercore::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::critical, __VA_ARGS__)