#pragma once
#include "NetworkInterface.hpp"

namespace network
{
    class Session;
    class Acceptor;
    class SendContext;
    class NetworkEvent
    {
    public:
        NetworkEvent(NetworkEventType type)
            : _type(type)
        {

        }
        virtual ~NetworkEvent() = default;

    public:
        void Initialize()
        {
            _owner.reset();
        }

    public:
        NetworkEventType                    GetNetworkEventType() const { return _type; }
        std::shared_ptr<INetworkObject>     GetOwner() { return _owner; }

        void                                SetOwner(std::shared_ptr<INetworkObject> owner) { _owner = owner; }

    protected:
        NetworkEventType                    _type = NetworkEventType::None;
        std::shared_ptr<INetworkObject>     _owner;
    };

    class ConnectEvent : public NetworkEvent
    {
    public:
        ConnectEvent()
            : NetworkEvent(NetworkEventType::Connect)
        {

        }

    public:
        std::shared_ptr<Session> GetOwnerSession();
    };

    class DisconnectEvent : public NetworkEvent
    {
    public:
        DisconnectEvent()
            : NetworkEvent(NetworkEventType::Disconnect)
        {

        }

    public:
        std::shared_ptr<Session> GetOwnerSession();
    };

    class AcceptEvent : public NetworkEvent
    {
    public:
        AcceptEvent()
            : NetworkEvent(NetworkEventType::Accept)
        {

        }

    public:
        void                                SetAcceptSocketFd(SocketFd socket) { _acceptSocket = socket;}
        SocketFd&                           GetAcceptSocketFd() { return _acceptSocket; }
        std::shared_ptr<Acceptor>           GetOwnerAcceptor();

    private:
        SocketFd _acceptSocket = INVALID_SOCKET_FD_VALUE;
    };

    class SendEvent : public NetworkEvent
    {
    public:
        SendEvent()
            : NetworkEvent(NetworkEventType::Send)
        {

        }

    public:
        std::shared_ptr<Session>                      GetOwnerSession();
        std::vector<std::shared_ptr<SendContext>>&    GetSendContexts() { return _sendContexts; }
    private:
        std::vector<std::shared_ptr<SendContext>>     _sendContexts;
    };

    class RecvEvent : public NetworkEvent
    {
    public:
        RecvEvent()
            : NetworkEvent(NetworkEventType::Recv)
        {

        }

    public:
        std::shared_ptr<Session> GetOwnerSession();
    };

    class ErrorEvent : public NetworkEvent
    {
    public:
        ErrorEvent()
            : NetworkEvent(NetworkEventType::Error)
        {

        }
    };
}