#include "engine/Logger.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace engine
{
    std::shared_ptr<spdlog::logger>& Logger::GetInstance()
    {
        static std::shared_ptr<spdlog::logger> instance;
        return instance;
    }

    void Logger::SetPattern(const std::shared_ptr<spdlog::logger>& logger)
    {
        // 시간 + 레벨 + tid + 호출 위치(파일:라인 함수) + 메시지
        // %t  : thread id
        // %s  : source filename
        // %#  : source line
        // %!  : function name
        // %v  : message
        //
        // NOTE: %s/%#/%! 가 의미 있으려면 Log() 호출 시 source_loc가 전달되어야 함 (우린 매크로로 전달)

        logger->set_pattern(
            "%Y-%m-%d %H:%M:%S.%e "
            "[%^%l%$] "
            "[tid:%t] "
            "[%s:%# %!] "
            "%v"
        );

    }

    void Logger::Initialize(const std::string& loggerName,spdlog::level::level_enum level,size_t queueSize,spdlog::level::level_enum flushLevel)
    {
        // 1) 로그 전용 스레드 1개짜리 async thread pool 생성
        if (!spdlog::thread_pool())
        {
            spdlog::init_thread_pool(queueSize, 1); // worker=1 => 로그 전용 스레드 1개
        }

        // 2) sinks: 콘솔 + 회전 파일
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        std::vector<spdlog::sink_ptr> sinks{ consoleSink };

        // 3) async logger 생성
        // overflow_policy:
        // - block: 로그가 많으면 생산자 스레드가 잠깐 멈춤(유실 최소화, 디버깅/개발에 유리)
        // - overrun_oldest: 오래된 로그를 버리고 계속 진행
        auto asyncLogger = std::make_shared<spdlog::async_logger>(
            loggerName,
            sinks.begin(), sinks.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );

        asyncLogger->set_level(level);
        // logger->flush_on(flushLevel);
        SetPattern(asyncLogger);

        spdlog::set_default_logger(asyncLogger);
        GetInstance() = std::move(asyncLogger);

        SPDLOG_INFO("Logger initialized");
    }

    void Logger::Shutdown()
    {
        auto logger = GetInstance();
        if (logger) 
        {
            logger->flush();
        }

        // 스레드풀/로거 정리
        spdlog::shutdown();
        GetInstance().reset();
    }
}