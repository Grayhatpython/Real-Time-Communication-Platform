#pragma once
#include "NetworkEvent.hpp"

namespace servercore
{
    struct ControlEvents
    {
        EventFd removeSessionFd = INVALID_EVENT_FD_VALUE;
        EventFd shutdownFd = INVALID_EVENT_FD_VALUE;
    };

    class EpollDispatcher : public INetworkDispatcher
    {
        static constexpr size_t S_DEFALUT_EPOLL_EVENT_SIZE = 64;

    public:
        EpollDispatcher();
        virtual ~EpollDispatcher() override;
        
    public:
        bool Initialize();
        
    private:
        bool RegisterShutdownFd();
        bool RegisterRemoveSessionFd();

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
        void                    PostShutdownEvent();
        void                    PostSessionRemoveEvent();

    private:
        void                    PostEventSignal(EventFd controlEvent);

    public:
        EpollFd                 GetEpollFd() { return _epollFd; }

    private:
        EpollFd                         _epollFd = INVALID_EPOLL_FD_VALUE;
        std::vector<struct epoll_event> _epollEvents;
        ControlEvents                   _controlEvents;
    };
}