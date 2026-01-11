#pragma once
#include "StreamBuffer.hpp"
#include "NetworkAddress.hpp"
#include "NetworkInterface.hpp"

namespace servercore
{
    class Session : public EpollObject
    {
    public:
        Session();
        virtual ~Session() override;

    public:
        virtual NetworkObjectType   GetNetworkObjectType() override;
        virtual SocketFd            GetSocketFd() override;
        virtual void                Dispatch(INetworkEvent* networkEvent, bool succeeded, int32 errorCode) override;

    public:
        bool    Connect(NetworkAddress& targetNetworkAddress);
        void    Disconnect();
        bool    Send(std::shared_ptr<SendContext> sendContext);

    public:
        virtual void OnConnected() {};
        virtual void OnDisconnected() {};
        virtual void OnRecv(BYTE* buffer, int32 numOfBytes) {};
        virtual void OnSend() {};

    private:
   		void		RegisterConnect(ConnectEvent* connectEvent, NetworkAddress& targetAddress);
		void		RegisterDisconnect(DisconnectEvent* disconnectEvent);
		void		RegisterRecv();
		void		RegisterSend();

		void		ProcessConnect();
		void		ProcessConnect(ConnectEvent* connectEvent, int32 numOfBytes);
		void		ProcessDisconnect(DisconnectEvent* disconnectEvent, int32 numOfBytes);
		void		ProcessRecv(RecvEvent* recvEvent, int32 numOfBytes);
		void		ProcessSend(SendEvent* sendEvent, int32 numOfBytes);

		void		CloseSocket();

		void		HandleError(INetworkEvent* networkEvent, int32 errorCode);

	public:
		void								SetNetworkDispatcher(std::shared_ptr<INetworkDispatcher> networkDispatcher) { _networkDispatcher = networkDispatcher; }
		std::shared_ptr<INetworkDispatcher>	GetNetworkDispatcher() { return _networkDispatcher; }

		void				SetSocket(SocketFd socket) { _socket = socket; }
		SocketFd&			GetSocket() { return _socket; }

		void				SetRemoteAddress(const NetworkAddress& remoteAddress) { _remoteAddress = remoteAddress; }
		NetworkAddress		GetRemoteAddress() const { return _remoteAddress; }

		bool				IsConnected() const { return _isConnected.load(); }
		uint64				GetSessionId() const { return _sessionId; }

    private:
    	static std::atomic<uint64> S_GenerateSessionId;

        SocketFd    _socket = INVALID_SOCKET_FD_VALUE;
        uint64      _sessionId = 0;

        NetworkAddress                              _remoteAddress{};
        std::shared_ptr<INetworkDispatcher>         _networkDispatcher;

        std::atomic<bool> _isConnected = false;
        std::atomic<bool> _isConnectPending = false;
        std::atomic<bool> _isDisconnectPosted = false;

        std::queue<std::shared_ptr<SendContext>>	_sendContextQueue;
        Lock										_lock;
        std::atomic<bool>							_isSending = false;
        std::atomic<uint32>							_sendRegisterCount = 0;

        StreamBuffer								_streamBuffer{};
    };
}