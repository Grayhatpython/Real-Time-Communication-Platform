#pragma once
#include "NetworkPch.h"
#include "StreamBuffer.h"
#include "NetworkAddress.h"
#include "NetworkEvent.h"
#include "SendBuffer.h"

#include "engine/Lock.h"

namespace network
{
    class SessionRegistry;
    class EpollDispatcher;

    class Session : public std::enable_shared_from_this<Session>
    {
        friend class Acceptor;

    public:
        Session();
        ~Session();

    public:
        SocketFd                    GetSocketFd() const  { return _socketFd; }
        

    public:
        bool    TryFlushSend(std::shared_ptr<SendContext> sendContext);
        void    Disconnect();
        bool    TryConnect(std::shared_ptr<EpollDispatcher>& dispatcher);
        
        public:
        virtual void OnConnected() {};
        virtual void OnDisconnected() {};
        virtual void OnRecv(BYTE* buffer, int32 numOfBytes) {};
        virtual void OnSend() {};
        
        void                OnSendable(std::shared_ptr<EpollDispatcher>& dispatcher);
        DispatchResult		ProcessRecv(RecvEvent* recvEvent);
        void		        ProcessConnect();
        
    private:
        void        DisconnectOwner(std::shared_ptr<EpollDispatcher>& dispatcher);
        void        FlushSendOwner(std::shared_ptr<EpollDispatcher>& dispatcher);
        int32       PacketParsing(BYTE* buffer, int32 dataSize);

    private:
        //  결과값 기반으로 에러처리 상세히 !
        

        bool        GetSocketError();

	public:
        void                SetOwner(SessionRegistry* sessionRegistry) { _sessionRegistry = sessionRegistry; }
        SessionRegistry*    GetOwner() const { return _sessionRegistry; }

        void                SetShardID(int32 shardId) { _shardId = shardId; }
		void				SetSocketFd(SocketFd socketFd) { _socketFd = socketFd; }

		void				SetRemoteAddress(const NetworkAddress& remoteAddress) { _remoteAddress = remoteAddress; }
		NetworkAddress		GetRemoteAddress() const { return _remoteAddress; }

        void                SetState(SessionState state) { _state.store(state, std::memory_order_release); }
        SessionState		GetState() const { return _state.load(std::memory_order_acquire); }
        void                SetSessionId(uint64 sessionId) { _sessionId = sessionId; }
		uint64				GetSessionId() const { return _sessionId; }


    private:
        SessionRegistry*                            _sessionRegistry = nullptr;
        uint64                                      _sessionId = 0;
        NetworkAddress                              _remoteAddress{};
        SocketFd                                    _socketFd = INVALID_SOCKET_FD_VALUE;
        int32                                       _shardId = -1;    
        std::atomic<SessionState>                   _state = SessionState::Disconnected;

        alignas(CACHE_LINE_SIZE) std::atomic<bool>	_isSending = false;
        std::atomic<uint32>							_sendRegisterCount = 0;

        engine::Lock								_lock;
        std::queue<std::shared_ptr<SendContext>>	_sendContextQueue;
        StreamBuffer								_streamBuffer{};
    };

}