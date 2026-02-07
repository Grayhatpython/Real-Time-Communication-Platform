#pragma once
#include "EngineTLS.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

namespace engine
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

#define EN_LOG_TRACE(...)    engine::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::trace,     engine::LthreadState->threadName , __VA_ARGS__)
#define EN_LOG_DEBUG(...)    engine::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::debug,     engine::LthreadState->threadName , __VA_ARGS__)
#define EN_LOG_INFO(...)     engine::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::info,      engine::LthreadState->threadName , __VA_ARGS__)
#define EN_LOG_WARN(...)     engine::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::warn,      engine::LthreadState->threadName , __VA_ARGS__)
#define EN_LOG_ERROR(...)    engine::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::err,       engine::LthreadState->threadName , __VA_ARGS__)
#define EN_LOG_CRITICAL(...) engine::Logger::Log(spdlog::source_loc{__FILE__, __LINE__, __PRETTY_FUNCTION__}, spdlog::level::critical,  engine::LthreadState->threadName , __VA_ARGS__)