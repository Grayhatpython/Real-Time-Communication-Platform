#pragma once
#include "Enums.hpp"

namespace network
{
    class NetworkEvent;
    class INetworkObject;

    class INetworkObject : public std::enable_shared_from_this<INetworkObject>
    {
    public:
        virtual ~INetworkObject() = default;

    public:
        virtual NetworkObjectType   GetNetworkObjectType() = 0;
        virtual SocketFd            GetSocketFd() const = 0;
        virtual void                Dispatch(NetworkEvent* networkEvent) = 0;  
    };

    class IEpollObject : public INetworkObject
    {
    public:
        virtual ~IEpollObject() = default;

    public:
        virtual NetworkObjectType   GetNetworkObjectType() = 0;
        virtual SocketFd            GetSocketFd() const = 0;

    };

    class INetworkDispatcher  : public std::enable_shared_from_this<INetworkDispatcher>
    {
    public:
        virtual ~INetworkDispatcher() = default;

    public:
        virtual bool Register(std::shared_ptr<INetworkObject> networkObject) = 0;
        virtual DispatchResult Dispatch(uint32 timeoutMs = TIMEOUT_INFINITE) = 0;
    };
}