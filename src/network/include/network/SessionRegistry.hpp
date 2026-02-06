#pragma once
#include <cstdint>
#include <functional>
#include <memory>

namespace network
{
    class Session;
    class ISessionRegistry 
    {
    public:
        virtual ~ISessionRegistry() = default;

        virtual void AddSession(std::shared_ptr<Session> session) = 0;
        virtual std::shared_ptr<Session> CreateSession() = 0;

        virtual void Clear() = 0;

        virtual void PushRemoveSessionEvent(uint64_t sessionId) = 0;
        // "Dispatch 스레드에서" 호출될 예정
        virtual void ProcessRemoveSessionEvent() = 0; 

        virtual void AbortSession(uint64_t sessionId) = 0;

        virtual void SetSessionFactory(std::function<std::shared_ptr<Session>()> f) = 0;
    };
}