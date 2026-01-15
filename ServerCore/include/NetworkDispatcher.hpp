#pragma once
#include "NetworkEvent.hpp"

namespace servercore
{
    class EpollDispatcher : public INetworkDispatcher
    {
        static constexpr size_t S_DEFALUT_EPOLL_EVENT_SIZE = 64;

    public:
        EpollDispatcher();
        virtual ~EpollDispatcher() override;
        
    public:
        bool Initialize();
        
    public:
        virtual bool            Register(std::shared_ptr<INetworkObject> networkObject) override;
        virtual DispatchResult  Dispatch(uint32 timeoutMs = TIMEOUT_INFINITE) override;
        virtual void            PostExitSignal() override;

        bool                    EnableSendEvent(std::shared_ptr<INetworkObject> networkObject);
        bool                    DisableSendEvent(std::shared_ptr<INetworkObject> networkObject);
        bool                    UnRegister(std::shared_ptr<INetworkObject> networkObject);

    public:
        EpollFd                 GetEpollFd() { return _epollFd; }

    private:
        EpollFd                         _epollFd = INVALID_EPOLL_FD_VALUE;
        std::vector<struct epoll_event> _epollEvents;
        FileDescriptor                  _exitSignalEventPipe[2] = { INVALID_FILE_DESCRIPTOR_VALUE, INVALID_FILE_DESCRIPTOR_VALUE};
    };
}