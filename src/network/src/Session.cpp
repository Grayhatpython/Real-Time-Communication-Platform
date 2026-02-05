#include "NetworkPch.hpp"
#include "Session.hpp"
#include "NetworkUtils.hpp"
#include "SessionManager.hpp"
#include "NetworkDispatcher.hpp"


namespace network
{
	std::atomic<uint64> Session::S_GenerateSessionId = 1;

	Session::Session()
	{
		_sessionId = S_GenerateSessionId.fetch_add(1);
	}

	Session::~Session()
	{
		
		//	TEMP
		while (_sendContextQueue.empty() == false)
			_sendContextQueue.pop();
	}

	bool Session::Connect(NetworkAddress& targetAddress)
	{
		SessionState state = GetState();
		if (state != SessionState::Disconnected)
		{
			return false;
		}

		_socketFd = NetworkUtils::CreateSocketFd(true);
		if (_socketFd == INVALID_SOCKET_FD_VALUE)
		{
			return false;
		}

		if (NetworkUtils::Bind(_socketFd, static_cast<uint16>(0)) == false)
		{
			NetworkUtils::CloseSocketFd(_socketFd);
			return false;
		}

		struct sockaddr_in serverAddress = targetAddress.GetSocketAddress();
		_state.store(SessionState::ConnectPending, std::memory_order_release);

		int32 ret = ::connect(_socketFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

		if(ret == RESULT_OK)
		{
			if(_networkDispatcher->Register(shared_from_this()) == false)
			{
				NetworkUtils::CloseSocketFd(_socketFd);
				_state.store(SessionState::Disconnected, std::memory_order_release);
				return false;
			}

			ProcessConnect();
		}

		if(ret == RESULT_ERROR)
		{
			//	EINPROGRESS 경우 연결이 즉시 완료되지 않았지만, 백그라운드에서 진행 중
			// 	이 경우 이후 epoll 등으로 EPOLLOUT 이벤트를 기다려 연결 완료를 감지
			//	그 외 음수 값 (다른 errno): 연결 시도가 명백한 오류로 실패
			if(errno != EINPROGRESS)
			{
				NetworkUtils::CloseSocketFd(_socketFd);
				_state.store(SessionState::Disconnected, std::memory_order_release);
				return false;
			}

			return RegisterAsyncConnect();
		}

		return true;
	}

	void Session::Disconnect()
	{
		SessionState expected = SessionState::Connected;
		
		if(_state.compare_exchange_strong(expected, SessionState::Disconnected, std::memory_order_acq_rel, std::memory_order_acquire))
		{	
			DisconnectEvent* disconnectEvent = cnew<DisconnectEvent>();
			auto session = shared_from_this();
			disconnectEvent->SetOwner(session);

			CloseSocket();
			
			ProcessDisconnect(disconnectEvent);
		}
		else
		{
			;
		}
	}

	bool Session::TryFlushSend(std::shared_ptr<SendContext> sendContext)
	{
		if(GetState() != SessionState::Connected)
			return false;

		{
			WriteLockGuard lock(_lock);
			_sendContextQueue.push(sendContext);
		}

		bool expected = false;
		if(_isSending.compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed) == true)
		{
			FlushSend();
			return true;
		}

		return true;
	}

	void Session::FlushSend()
	{
		auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);

		std::queue<std::shared_ptr<SendContext>> sendContexts;
		{
			WriteLockGuard lock(_lock);
			
			sendContexts.swap(_sendContextQueue);
			
		}

		while(sendContexts.empty() == false)
		{
			auto sendContext = sendContexts.front();

			const BYTE* baseSendBufferPtr = sendContext->sendBuffer->GetBuffer();
			const size_t remainPacketSize = sendContext->size - sendContext->offset;

			if(sendContext->offset >= sendContext->size)
			{
				sendContexts.pop();
				continue;;
			}

			sendContext->iovecBuf.iov_base =  const_cast<BYTE*>(baseSendBufferPtr + sendContext->offset);
 			sendContext->iovecBuf.iov_len  = remainPacketSize;

			msghdr msg{};
			msg.msg_iov = &sendContext->iovecBuf;
			msg.msg_iovlen = 1;

			ssize_t bytesTransferred = ::sendmsg(_socketFd, &msg, MSG_NOSIGNAL);
			if(bytesTransferred > 0)
			{
				sendContext->offset += static_cast<size_t>(bytesTransferred);

				if(sendContext->offset >= sendContext->size)
				{
					sendContexts.pop();
				}

				continue;
			}

			if(bytesTransferred == 0)
			{
				epollDispatcher->DisableConnectEvent(shared_from_this());
				Disconnect();
				break;
			}

			if(errno == EINTR)
				continue;

			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				epollDispatcher->EnableConnectEvent(shared_from_this());
				break;
			}	

			epollDispatcher->DisableConnectEvent(shared_from_this());
			Disconnect();
			break;
		}

		_isSending.store(false, std::memory_order_release);
	}

	bool Session::RegisterAsyncConnect()
	{
		if(_networkDispatcher == nullptr)
			return false;

		if(_networkDispatcher->Register(shared_from_this()) == false)
		{
			NetworkUtils::CloseSocketFd(_socketFd);
			_state.store(SessionState::Disconnected, std::memory_order_release);
			return false;
		}

		auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
		if(epollDispatcher == nullptr)
		{
			CloseSocket();
			_state.store(SessionState::Disconnected, std::memory_order_release);
			return false;
		}

		if(epollDispatcher->EnableConnectEvent(shared_from_this()) == false)
		{
			CloseSocket();
			_state.store(SessionState::Disconnected, std::memory_order_release);
			return false;
		}

		return true;
	}

	//	Server Accept4() -> ProcessConnect
	void Session::ProcessConnect()
	{
		_state.store(SessionState::Connected, std::memory_order_release);

		//	Server Client Contents 
		OnConnected();

        NC_LOG_INFO("{} Session is Connected", _sessionId);
	}

	//	Client Connect() -> ProcessConnect
	void Session::ProcessConnect(ConnectEvent* connectEvent)
	{
		if(connectEvent)
		{
			cdelete(connectEvent);
			connectEvent = nullptr;
		}

		//	TODO
		if(GetSocketError() == false)
		{
			Disconnect();
			return;
		}

		//	network dispatcher가 등록되지 않은 경우 정상 통신이 불가능하므로 제거
		if(_networkDispatcher == nullptr)
		{
			NetworkUtils::CloseSocketFd(_socketFd);
			GSessionManager->AbortSession(_sessionId);
		}

		
		auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
		if(epollDispatcher)
		{
			if(epollDispatcher->DisableConnectEvent(shared_from_this()) == false)
			{
				Disconnect();
				return;
			}
		}

		_state.store(SessionState::Connected, std::memory_order_release);

		OnConnected();

		NC_LOG_INFO("Connected to the {} session(server)", _sessionId);
	}

	void Session::ProcessDisconnect(DisconnectEvent* disconnectEvent)
	{	
		OnDisconnected();

		if(disconnectEvent)
		{
			cdelete(disconnectEvent);
			disconnectEvent = nullptr;
		}

		//	TODO
		auto session = std::static_pointer_cast<Session>(shared_from_this());
		if(session)
		{
			auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
			if(epollDispatcher)
			{
				epollDispatcher->PostRemoveSessionEvent(_sessionId);
			}
		}

		NC_LOG_INFO("{} session Disconnected", _sessionId);
	}

	void Session::ProcessRecv(RecvEvent* recvEvent)
	{
		if(GetState() != SessionState::Connected)
		{
			;
		}

		while(true)
		{
			int32 recvLen = ::recv(_socketFd, _streamBuffer.GetWritePos(), _streamBuffer.GetWriteableSize(), 0);
			
			if(recvLen < 0)
			{
				//	더 이상 읽을 데이터 없음
				if(errno == EAGAIN || errno == EWOULDBLOCK)
					break;

				//	error
				break;
			}
			else if(recvLen == 0)
			{
				Disconnect();
				break;
			}

			//	처리된 데이터 크기만큼 streamBuffer writePos 처리
			if (_streamBuffer.OnWrite(recvLen) == false)
			{
				//	정해진 용량 초과 -> 연결 종료 
				Disconnect();
				return;
			}

			auto dataSize = _streamBuffer.GetReadableSize();

			auto processLen = PacketParsing(_streamBuffer.GetReadPos(), dataSize);

			//	streamBuffer ReadPos 처리
			if (processLen < 0 || processLen > dataSize || _streamBuffer.OnRead(processLen) == false)
			{
				//	??? 로직상 오면 안되는 부분 연결 종료
				Disconnect();
				return;
			}
			
			_streamBuffer.Clean();
		}

		if(recvEvent)
		{
			cdelete(recvEvent);
			recvEvent = nullptr;
		}
	}

	int32 Session::PacketParsing(BYTE* buffer, int32 dataSize)
	{
		int32 processLen = 0;

		//	패킷 파싱 처리
		while(true)
		{
			const int32 readableSize = dataSize - processLen;

			//	최소한 헤더 크기만큼의 데이터가 없으면 파싱 하지 않음
			if (readableSize < sizeof(PacketHeader))
				break;

			//	헤더를 읽어서 전체 패킷 크기 확인
			BinaryReader br(buffer, sizeof(PacketHeader));
			uint16 packetSize = 0;
			br.Read<uint16>(packetSize);

			// PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(&buffer[processLen]);

			//	확인한 패킷 크기가 패킷 사이즈보다 작다면
			if (readableSize < packetSize)
			{
				break;
			}

			//	컨텐츠 영역 ( 서버 or 클라이언트 ) 에서 해당 패킷에 대한 로직 처리
			OnRecv(&buffer[processLen], packetSize);

			processLen += packetSize;
		}

		return processLen;
	}

	void Session::ProcessSend(SendEvent* sendEvent)
	{
		if(GetState() != SessionState::Connected)
			;

		bool expected = false;
		if(_isSending.compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed) == true)
		{
			FlushSend();
		}
	}

    void Session::ProcessError(ErrorEvent *errorEvent)
    {
		if(GetSocketError() == true)
		{
			//	?
		}

		Disconnect();
    }

    void Session::CloseSocket()
    {
		if(_networkDispatcher)
		{
			auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
			if(epollDispatcher)
				epollDispatcher->UnRegister(shared_from_this());
		}

		if(_socketFd != INVALID_SOCKET_FD_VALUE)
		{
			NetworkUtils::CloseSocketFd(_socketFd);
			_socketFd = INVALID_SOCKET_FD_VALUE;
		}			
    }

    bool Session::GetSocketError()
    {
       	int32 err = 0;
		socklen_t len = sizeof(err);
		if (::getsockopt(_socketFd, SOL_SOCKET, SO_ERROR, &err, &len) == RESULT_ERROR)
			return false;

		if (err != RESULT_OK)
		{
			return false;
		}

		return true;
    }

    void Session::Dispatch(INetworkEvent* networkEvent)
	{
		if(networkEvent)
		{
			switch (networkEvent->GetNetworkEventType())
			{
			case NetworkEventType::Connect:
				{
					ConnectEvent* connectEvent = static_cast<ConnectEvent*>(networkEvent);
					if(connectEvent)
					{
						auto networkObject = shared_from_this();
						connectEvent->SetOwner(networkObject);
						ProcessConnect(connectEvent);
					}
				}
				break;
			case NetworkEventType::Recv:
			{
				RecvEvent* recvEvent = static_cast<RecvEvent*>(networkEvent);
				if(recvEvent)
				{
					auto networkObject = shared_from_this();
					recvEvent->SetOwner(networkObject);
					ProcessRecv(recvEvent);
				}
				break;
			}
			case NetworkEventType::Send:
			{
				SendEvent* sendEvent = static_cast<SendEvent*>(networkEvent);
				if(sendEvent)
				{
					auto networkObject = shared_from_this();
					sendEvent->SetOwner(networkObject);
					ProcessSend(sendEvent);
				}
				break;
			}
			case NetworkEventType::Error:
			{
				ErrorEvent* errorEvent = static_cast<ErrorEvent*>(networkEvent);
				if(errorEvent)
				{
					auto networkObject = shared_from_this();
					errorEvent->SetOwner(networkObject);
					ProcessError(errorEvent);
				}
				break;
			}
			default:
				//	???
				break;
			}	
		}
	}

}

         