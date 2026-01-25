#pragma once

namespace servercore
{
    class INetworkObject;
    class INetworkEvent
    {
    public:
        INetworkEvent(NetworkEventType type)
            : _type(type)
        {

        }
        virtual ~INetworkEvent() = default;

    public:
        NetworkEventType                    GetNetworkEventType() const { return _type; }
        std::shared_ptr<INetworkObject>     GetOwner() { return _owner; }

        void                                SetOwner(std::shared_ptr<INetworkObject> owner) { _owner = owner; }

    protected:
        NetworkEventType                    _type = NetworkEventType::None;
        std::shared_ptr<INetworkObject>     _owner;
    };

    class INetworkObject : public std::enable_shared_from_this<INetworkObject>
    {
    public:
        virtual ~INetworkObject() = default;

    public:
        virtual NetworkObjectType   GetNetworkObjectType() = 0;
        virtual SocketFd            GetSocketFd() const = 0;
        virtual void                Dispatch(INetworkEvent* networkEvent) = 0;  
    };

    class EpollObject : public INetworkObject
    {
    public:
        virtual ~EpollObject() = default;

    public:
        virtual NetworkObjectType   GetNetworkObjectType() = 0;
        virtual SocketFd            GetSocketFd() const = 0;
        virtual void                Dispatch(INetworkEvent* networkEvent) = 0;
    };

    class INetworkDispatcher  : public std::enable_shared_from_this<INetworkDispatcher>
    {
    public:
        virtual ~INetworkDispatcher() = default;

    public:
        virtual bool Register(std::shared_ptr<INetworkObject> networkObject) = 0;
        virtual DispatchResult Dispatch(uint32 timeoutMs = TIMEOUT_INFINITE) = 0;
    };

    class SendBuffer;
    struct SendContext
    {
        std::shared_ptr<SendBuffer> sendBuffer;
        struct iovec iovecBuf{};
    };
    
}