#pragma once
#include "StreamBuffer.hpp"
#include "NetworkAddress.hpp"
#include "NetworkEvent.hpp"
#include "SendBuffer.hpp"

constexpr size_t CACHE_LINE_SIZE = 64;

namespace servercore
{
    struct SessionIdentity
    {
        SocketFd                    socketFd = INVALID_SOCKET_FD_VALUE;
        uint64                      sessionId = 0;
        std::atomic<SessionState>   state = SessionState::Disconnected;
    };

    alignas(64) struct SessionSendFlags
    {
        std::atomic<bool>   isSending{false};
        std::atomic<uint32> sendRegisterCount{0};
    };

    struct SessionIOBuffer
    {
        Lock                                        lock;
        std::queue<std::shared_ptr<SendContext>>    sendQueue;
        StreamBuffer                                streamBuffer;
    };

    struct SessionEnv
    {
        NetworkAddress                          remoteAddress;
        std::shared_ptr<INetworkDispatcher>     dispatcher;
    };

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
        void        FlushSend();
        int32       PacketParsing(BYTE* buffer, int32 dataSize);

    private:
        //  결과값 기반으로 에러처리 상세히 !
		void		ProcessConnect();
        void        ProcessConnect(ConnectEvent* connectEvent);
        
		void		ProcessDisconnect(DisconnectEvent* disconnectEvent);
		void		ProcessRecv(RecvEvent* recvEvent);
		void		ProcessSend(SendEvent* sendEvent);
        void        ProcessError(ErrorEvent* errorEvent);

        void        CloseSocket();

        bool        GetSocketError();

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

        SocketFd                                    _socketFd = INVALID_SOCKET_FD_VALUE;
        NetworkAddress                              _remoteAddress{};
        std::shared_ptr<INetworkDispatcher>         _networkDispatcher;
        uint64                                      _sessionId = 0;
        std::atomic<SessionState>                   _state = SessionState::Disconnected;

        alignas(64) std::atomic<bool>				_isSending = false;
        std::atomic<uint32>							_sendRegisterCount = 0;

        Lock										_lock;
        std::queue<std::shared_ptr<SendContext>>	_sendContextQueue;
        StreamBuffer								_streamBuffer{};
    };

}