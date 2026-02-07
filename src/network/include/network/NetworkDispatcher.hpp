#pragma once
#include "NetworkEvent.hpp"

namespace network
{

    class SessionRegistry;
    class EpollDispatcher : public INetworkDispatcher
    {
        static constexpr size_t S_DEFALUT_EPOLL_EVENT_SIZE = 128;

    public:
        EpollDispatcher();
        virtual ~EpollDispatcher() override;
        
    public:
        bool Initialize();
        void Stop();
        void Run(std::stop_token st);
        
    private:
        bool RegisterWakeupFd();

    public:
        virtual bool            Register(std::shared_ptr<INetworkObject> networkObject) override;
        virtual DispatchResult  Dispatch(uint32 timeoutMs = TIMEOUT_INFINITE) override;
       
        bool                    EnableConnectEvent(std::shared_ptr<INetworkObject> networkObject);
        bool                    DisableConnectEvent(std::shared_ptr<INetworkObject> networkObject);
        bool                    EnableSendEvent(std::shared_ptr<INetworkObject> networkObject);
        bool                    DisableSendEvent(std::shared_ptr<INetworkObject> networkObject);
        bool                    UnRegister(std::shared_ptr<INetworkObject> networkObject);

    private:
        bool                    EnableEvent(const std::shared_ptr<INetworkObject>& networkObject);
        bool                    DisableEvent(const std::shared_ptr<INetworkObject>& networkObject);
        
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

    private:
        EpollFd                         _epollFd = INVALID_EPOLL_FD_VALUE;
        std::vector<struct epoll_event> _epollEvents;
        EventFd                         _wakeupFd = INVALID_EVENT_FD_VALUE;

        int32                           _shardId = 0;

        SessionRegistry*                _sessionRegistry = nullptr;
        std::atomic<bool>               _running{true};
    };
}