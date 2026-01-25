#pragma once
#include "StreamBuffer.hpp"
#include "NetworkAddress.hpp"
#include "NetworkEvent.hpp"

namespace servercore
{

    class Session : public EpollObject
    {
        friend class Acceptor;
        friend class Client;

    public:
        Session();
        virtual ~Session() override;

    public:
		virtual NetworkObjectType   GetNetworkObjectType() override { return NetworkObjectType::Session; }
        virtual SocketFd            GetSocketFd() const override { return _socketFd; }
        virtual void                Dispatch(INetworkEvent* networkEvent) override;

    public:
        bool    Connect(NetworkAddress& targetNetworkAddress);
        void    Disconnect();
        bool    TryFlushSend(std::shared_ptr<SendContext> sendContext);

    public:
        virtual void OnConnected() {};
        virtual void OnDisconnected() {};
        virtual void OnRecv(BYTE* buffer, int32 numOfBytes) {};
        virtual void OnSend() {};

    private:
        bool        RegisterAsyncConnect();

    private:
        //  결과값 기반으로 에러처리 상세히 !
		void		ProcessConnect();
        void        ProcessConnect(ConnectEvent* connectEvent);
        bool        QueryConnectError();
        
		void		ProcessDisconnect(DisconnectEvent* disconnectEvent);
		void		ProcessRecv(RecvEvent* recvEvent);
		void		ProcessSend(SendEvent* sendEvent);

        void        CloseSocket();

	public:
		void								SetNetworkDispatcher(std::shared_ptr<INetworkDispatcher> networkDispatcher) { _networkDispatcher = networkDispatcher; }
		std::shared_ptr<INetworkDispatcher>	GetNetworkDispatcher() { return _networkDispatcher; }

		void				SetSocketFd(SocketFd socketFd) { _socketFd = socketFd; }

		void				SetRemoteAddress(const NetworkAddress& remoteAddress) { _remoteAddress = remoteAddress; }
		NetworkAddress		GetRemoteAddress() const { return _remoteAddress; }

        SessionState		GetState() const { return _state.load(std::memory_order_acquire); }
		uint64				GetSessionId() const { return _sessionId; }

    private:
    	static std::atomic<uint64> S_GenerateSessionId;

        SocketFd    _socketFd = INVALID_SOCKET_FD_VALUE;
        uint64      _sessionId = 0;

        NetworkAddress                              _remoteAddress{};
        std::shared_ptr<INetworkDispatcher>         _networkDispatcher;

        std::atomic<SessionState>                   _state = SessionState::Disconnected;

        std::queue<std::shared_ptr<SendContext>>	_sendContextQueue;
        Lock										_lock;
        std::atomic<bool>							_isSending = false;
        std::atomic<uint32>							_sendRegisterCount = 0;

        StreamBuffer								_streamBuffer{};
    };

    #pragma pack(push, 1)
    struct PacketHeader
    {
        uint16 size;
        uint16 id;
    };
    #pragma pack(pop)
}