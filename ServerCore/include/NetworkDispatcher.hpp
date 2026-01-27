#pragma once
#include "NetworkEvent.hpp"

namespace servercore
{
    struct CoreEvent
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
        void Stop();
        
    private:
        bool RegisterShutdownFd();
        bool RegisterRemoveSessionFd();

    public:
        virtual bool            Register(std::shared_ptr<INetworkObject> networkObject) override;
        virtual DispatchResult  Dispatch(uint32 timeoutMs = TIMEOUT_INFINITE) override;
        
        /*
        template<typename... Args>
        void                    PostEventSignal(CoreEventType type, Args&&... args)
        {
            switch (type)
            {
            case CoreEventType::CoreShutdown:
                PostRemoveSessionEvent(_coreEvents.shutdownFd, std::forward<Args>(args)...);
                break;
            case CoreEventType::SessionRemove:
                PostCoreShutdown(_coreEvents.removeSessionFd);
                break;
            default:
                break;
            }
        }
        */
       
        bool                    EnableConnectEvent(std::shared_ptr<INetworkObject> networkObject);
        bool                    DisableConnectEvent(std::shared_ptr<INetworkObject> networkObject);
        bool                    EnableSendEvent(std::shared_ptr<INetworkObject> networkObject);
        bool                    DisableSendEvent(std::shared_ptr<INetworkObject> networkObject);
        bool                    UnRegister(std::shared_ptr<INetworkObject> networkObject);

    private:
        bool                    EnableEvent(const std::shared_ptr<INetworkObject>& networkObject);
        bool                    DisableEvent(const std::shared_ptr<INetworkObject>& networkObject);
        
    public:
        void                    PostRemoveSessionEvent(uint64 sessionId);
        void                    PostCoreShutdown();
        
    private:
        void                    PostEventSignal(EventFd coreEventFd);

    private:
        void                    ConsumeEventSignal(CoreEventType type);                   
        void                    ConsumeEventSignal(EventFd coreEventFd);

    public:
        EpollFd                 GetEpollFd() { return _epollFd; }

    private:
        EpollFd                         _epollFd = INVALID_EPOLL_FD_VALUE;
        std::vector<struct epoll_event> _epollEvents;
        CoreEvent                       _coreEvents;
    };
}