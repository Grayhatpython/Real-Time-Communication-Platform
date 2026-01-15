#include "Pch.hpp"
#include "Session.hpp"
#include "NetworkDispatcher.hpp"
#include "NetworkUtils.hpp"
#include "SessionManager.hpp"

namespace servercore
{
	std::atomic<uint64> Session::S_GenerateSessionId = 1;

	Session::Session()
	{
		_sessionId = S_GenerateSessionId.fetch_add(1);
	}

	Session::~Session()
	{
		CloseSocket();
		
		//	TEMP
		while (_sendContextQueue.empty() == false)
			_sendContextQueue.pop();
	}

	bool Session::Connect(NetworkAddress& targetAddress)
	{
		if (_isConnected.load() == true || _isConnectPending.load() == true)
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
			NetworkUtils::CreateSocketFd(_socketFd);
			return false;
		}

		if (_networkDispatcher->Register(shared_from_this()) == false)
		{
			NetworkUtils::CreateSocketFd(_socketFd);
			return false;
		}

		struct sockaddr_in serverAddress = targetAddress.GetSocketAddress();
		_isConnectPending.store(true);

		int32 error = ::connect(_socketFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
		if(error == RESULT_ERROR)
		{
			//	EINPROGRESS 경우 연결이 즉시 완료되지 않았지만, 백그라운드에서 진행 중
			// 	이 경우 이후 epoll 등으로 EPOLLOUT 이벤트를 기다려 연결 완료를 감지
			//	그 외 음수 값 (다른 errno): 연결 시도가 명백한 오류로 실패
			if(errno != EINPROGRESS)
			{
				NetworkUtils::CreateSocketFd(_socketFd);
				_isConnectPending.store(false);
				return false;
			}
		}
		else
		{
			ConnectEvent* connectEvent = cnew<ConnectEvent>();
			//	바로 Connect
			// ProcessConnect(connectEvent);
		}

		return true;
	}

	void Session::Disconnect()
	{
		if (_isConnected.load() == false || _isDisconnectPosted.exchange(true))
			return;

		DisconnectEvent* disconnectEvent = cnew<DisconnectEvent>();
		auto session = shared_from_this();
		disconnectEvent->SetOwner(session);
		ProcessDisconnect(disconnectEvent);
	}

	bool Session::TryFlushSend(std::shared_ptr<SendContext> sendContext)
	{
		if(_isConnected.load() == false)
			return false;

		/* 
		[게임 로직이 보내야 할 데이터 발생]
		TryFlushSend() 호출
		send()로 가능한 만큼 보냄
		EAGAIN(버퍼 꽉참) -> EPOLLOUT 감시 켬(여유 생기면 알려달라고)		
		(시간 지나 송신버퍼에 여유 생김)
		epoll_wait에서 EPOLLOUT 이벤트 도착
		다시 TryFlushSend() 호출
		다 보낼 때까지 반복, 끝나면 EPOLLOUT 끔 

		전담 send 스레드 활용할 생각
		*/

	}

	void Session::ProcessConnect()
	{
		//	accept -> 
		OnConnected();
	}

	void Session::ProcessDisconnect(DisconnectEvent* disconnectEvent)
	{
		_isConnected.store(false);

		OnDisconnected();

		CloseSocket();

		if(disconnectEvent)
		{
			cdelete(disconnectEvent);
			disconnectEvent = nullptr;
		}

		GSessionManager->RemoveSession()
	}

	void Session::ProcessRecv(RecvEvent* recvEvent)
	{
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

			//	패킷 파싱 처리
			while(true)
			{
				//	처리된 데이터 크기만큼 streamBuffer writePos 처리
				if (_streamBuffer.OnWrite(recvLen) == false)
				{
					//	정해진 용량 초과 -> 연결 종료 
					Disconnect();
					return;
				}

				const int32 readableSize = _streamBuffer.GetReadableSize();

				//	최소한 헤더 크기만큼의 데이터가 없으면 파싱 하지 않음
				if (readableSize < sizeof(PacketHeader))
					break;

				//	헤더를 읽어서 전체 패킷 크기 확인
				PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(_streamBuffer.GetReadPos());
				const uint16 packetSize = packetHeader->size;

				//	확인한 패킷 크기가 패킷 헤더보다 작다면 -> ?? 로직에 벗어난 패킷임
				if (packetSize < sizeof(PacketHeader))
				{
					//	비정상적인 패킷 크기 연결 종료
					Disconnect();
					break;
				}

				//	패킷 하나만큼의 사이즈를 읽을 수 있다면 -> 완성된 하나의 패킷을 읽을 수 있다면
				if (readableSize < packetSize)
					break;

				//	컨텐츠 영역 ( 서버 or 클라이언트 ) 에서 해당 패킷에 대한 로직 처리
				OnRecv(_streamBuffer.GetReadPos(), packetSize);

				//	streamBuffer ReadPos 처리
				if (_streamBuffer.OnRead(packetSize) == false)
				{
					//	??? 로직상 오면 안되는 부분 연결 종료
					Disconnect();
					return;
				}
			}

			_streamBuffer.Clean();
		}

		if(recvEvent)
		{
			cdelete(recvEvent);
			recvEvent = nullptr;
		}
	}

	void Session::ProcessSend(SendEvent* sendEvent)
	{

	}

	void Session::CloseSocket()
	{
		auto linuxEpollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
		if(linuxEpollDispatcher->UnRegister(shared_from_this()) == false)
			;	//	???

		NetworkUtils::CloseSocketFd(_socketFd);
	}

	void Session::Dispatch(INetworkEvent* networkEvent)
	{
		if(networkEvent)
		{
			switch (networkEvent->GetNetworkEventType())
			{
			case NetworkEventType::Connect:
				break;
			case NetworkEventType::Recv:
				RecvEvent* recvEvent = static_cast<RecvEvent*>(networkEvent);
				if(recvEvent)
				{
					auto networkObject = shared_from_this();
					recvEvent->SetOwner(networkObject);
					ProcessRecv(recvEvent);
				}
				break;
			case NetworkEventType::Send:
				SendEvent* sendEvent = static_cast<SendEvent*>(networkEvent);
				if(sendEvent)
				{
					auto networkObject = shared_from_this();
					sendEvent->SetOwner(networkObject);
					ProcessSend(sendEvent);
				}
				break;
			case NetworkEventType::Error:
			default:
				//	???
				break;
			}	
		}
	}

}