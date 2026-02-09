#pragma once
#include "NetworkEvent.h"

namespace network
{
    class Session;
    class SessionRegistry;
    class EpollDispatcher : public std::enable_shared_from_this<EpollDispatcher>
    {
        static constexpr size_t S_DEFALUT_EPOLL_EVENT_SIZE = 128;

    public:
        EpollDispatcher();
        ~EpollDispatcher();
        
    public:
        bool Initialize();
        void Stop();
        void Run(std::stop_token st);
        
    private:
        bool RegisterWakeupFd();
        void Close();

    public:
        bool                    Register(std::shared_ptr<Session> session);
        
        bool                    EnableConnectEvent(std::shared_ptr<Session> session);
        bool                    DisableConnectEvent(std::shared_ptr<Session> session);
        bool                    EnableSendEvent(std::shared_ptr<Session> session);
        bool                    DisableSendEvent(std::shared_ptr<Session> session);
        bool                    UnRegister(std::shared_ptr<Session> session);

    private:
        bool                    EnableEvent(const std::shared_ptr<Session>& session);
        bool                    DisableEvent(const std::shared_ptr<Session>& session);
        
    public:
        void                    PostWakeup();

    private:
        void                    PostEventSignal(EventFd coreEventFd);

    private:               
        void                    ConsumeEventSignal(EventFd coreEventFd);

    public:
        EpollFd                                 GetEpollFd() { return _epollFd; }
        void									SetSessionRegistry(SessionRegistry*  sessionRegistry) { _sessionRegistry = sessionRegistry; }
		SessionRegistry*  						GetSessionRegistry() { return _sessionRegistry; }

        void                                    SetShardId(int32 shardId) { _shardId = shardId; }
        int32                                   GetShardId() const { return _shardId; }

        void                                    AddPendingCloseSession(uint64 sessionId) { _pendingCloseSessions.push_back(sessionId); } 

    private:
        EpollFd                         _epollFd = INVALID_EPOLL_FD_VALUE;
        std::vector<struct epoll_event> _epollEvents;
        EventFd                         _wakeupFd = INVALID_EVENT_FD_VALUE;

        int32                           _shardId = 0;

        SessionRegistry*                _sessionRegistry = nullptr;
        std::atomic<bool>               _running{true};

        std::vector<uint64>             _pendingCloseSessions;
    };
}