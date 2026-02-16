#include "network/NetworkPch.h"
#include "network/Session.h"
#include "network/NetworkUtils.h"
#include "network/SessionRegistry.h"
#include "network/NetworkDispatcher.h"

#include "network/PacketHeader.h"

#include "engine/Logger.h"
#include "engine/MemoryPool.h"
#include "engine/BinaryReader.h"

namespace network
{

	Session::Session()
	{
	}

	Session::~Session()
	{
		EN_LOG_DEBUG("~Session");

		//	TEMP
		while (_sendContextQueue.empty() == false)
			_sendContextQueue.pop();
	}


	bool Session::TryFlushSend(std::shared_ptr<SendContext> sendContext)
	{
		if (GetState() != SessionState::Connected)
        {
			Disconnect();
			return false;
		}

		{
			engine::WriteLockGuard lock(_lock);
			_sendContextQueue.push(std::move(sendContext));
		}

		// “오너 flush 스케줄”은 딱 한 번만
		bool wasSending = _isSending.exchange(true, std::memory_order_acq_rel);
		if (wasSending == false)
		{
			// 오너 shard로 flush 커맨드 post (eventfd로 깨움)
			std::weak_ptr<Session> sessionWeakPtr = shared_from_this();
			_sessionRegistry->PostToShard(_shardId, [sessionWeakPtr](SessionRegistry::Shard& shard, std::shared_ptr<EpollDispatcher>& dispatcher) {
				if (auto session = sessionWeakPtr.lock())
				{
					session->FlushSendOwner(dispatcher);
				}
			});
		}

		return true;
	}

    void Session::Disconnect()
    {
		// 오너 shard로 flush 커맨드 post (eventfd로 깨움)
		std::weak_ptr<Session> sessionWeakPtr = shared_from_this();
		_sessionRegistry->PostToShard(_shardId, [sessionWeakPtr](SessionRegistry::Shard& shard, std::shared_ptr<EpollDispatcher>& dispatcher) {
			if (auto session = sessionWeakPtr.lock())
			{
				session->DisconnectOwner(dispatcher);
			}
		});
    }

	bool Session::TryConnect(std::shared_ptr<EpollDispatcher>& dispatcher)
	{
		if (GetState() != SessionState::ConnectPending)
			return false;

		if(GetSocketError() == false)
			return false;

		auto session = shared_from_this();
		dispatcher->Register(session);

		ProcessConnect();

		// 연결되자마자 대기 send 있으면 바로 flush -> ?
		// FlushSendOwner(dispatcher);

		return true;
	}

	void Session::DisconnectOwner(std::shared_ptr<EpollDispatcher>& dispatcher)
	{
		dispatcher->AddPendingCloseSession(_sessionId);
		SetState(SessionState::DisconnectPosted);
	}

	void Session::FlushSendOwner(std::shared_ptr<EpollDispatcher>& dispatcher)
	{
		std::queue<std::shared_ptr<SendContext>> sendContexts;
		{
			engine::WriteLockGuard lock(_lock);
			sendContexts.swap(_sendContextQueue);
			
		}

		while(sendContexts.empty() == false)
		{
			auto sendContext = sendContexts.front();

			if(sendContext->offset >= sendContext->size)
			{
				sendContexts.pop();
				continue;;
			}

			const BYTE* baseSendBufferPtr = sendContext->sendBuffer->GetBuffer();
			const size_t remainPacketSize = sendContext->size - sendContext->offset;

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

			//	무슨 경우?
			if(bytesTransferred == 0)
			{
				dispatcher->DisableSendEvent(shared_from_this());
				dispatcher->AddPendingCloseSession(_sessionId);
				SetState(SessionState::DisconnectPosted);
				return;
			}

			if(errno == EINTR)
				continue;

			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				{
					engine::WriteLockGuard lock(_lock);
					while(sendContexts.empty() == false)
					{
						_sendContextQueue.push(sendContexts.front());
						sendContexts.pop();
					}
				}

				dispatcher->EnableSendEvent(shared_from_this());

				//	TODO
				return;
			}	

			dispatcher->DisableSendEvent(shared_from_this());
			dispatcher->AddPendingCloseSession(_sessionId);
			SetState(SessionState::DisconnectPosted);
			return;
		}

		dispatcher->DisableSendEvent(shared_from_this());
		_isSending.store(false, std::memory_order_release);

		//	찰나에 더 sendContextQueue에 send Context가 쌓여있을 경우 다시 스케쥴
		{
			bool hasMore = false;
			{
				engine::WriteLockGuard lock(_lock);
				hasMore = !_sendContextQueue.empty();
			}
			
			if (hasMore == true) 
			{
				bool expected = false;
				if (_isSending.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
				{
					// 오너 shard로 flush 커맨드 post (eventfd로 깨움)
					std::weak_ptr<Session> sessionWeakPtr = shared_from_this();
					_sessionRegistry->PostToShard(_shardId, [sessionWeakPtr](SessionRegistry::Shard& shard, std::shared_ptr<EpollDispatcher>& dispatcher) {
						if (auto session = sessionWeakPtr.lock())
						{
							session->FlushSendOwner(dispatcher);
						}
					});
				}
			}
		}
	}

	void Session::ProcessConnect()
	{
		_state.store(SessionState::Connected, std::memory_order_release);

		//	Server Client Contents 
		OnConnected();

        EN_LOG_INFO("{} Session is Connected", _sessionId);
	}


	DispatchResult Session::ProcessRecv(RecvEvent* recvEvent)
	{
		if(recvEvent)
		{
			engine::cdelete(recvEvent);
			recvEvent = nullptr;
		}

		if(GetState() != SessionState::Connected)
		{
			return DispatchResult::InvalidSessionStateProcessRecv;
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
				return DispatchResult::GracefulClose;
			}

			//	처리된 데이터 크기만큼 streamBuffer writePos 처리
			if (_streamBuffer.OnWrite(recvLen) == false)
			{
				//	정해진 용량 초과 -> 연결 종료 
				return DispatchResult::StreamBufferOverflow;
			}

			auto dataSize = _streamBuffer.GetReadableSize();

			auto processLen = PacketParsing(_streamBuffer.GetReadPos(), dataSize);

			//	streamBuffer ReadPos 처리
			if (processLen < 0 || processLen > dataSize || _streamBuffer.OnRead(processLen) == false)
			{
				//	??? 로직상 오면 안되는 부분 연결 종료
				return DispatchResult::InvalidPacketData;
			}
			
			_streamBuffer.Clean();
		}

		return DispatchResult::NetworkEventDispatched;
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
			engine::BinaryReader br(buffer, sizeof(PacketHeader));
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

	//	Owner 에서 하는거라 바로 pending 추가가능
	void Session::OnSendable(std::shared_ptr<EpollDispatcher>& dispatcher)
	{
		if(GetState() != SessionState::Connected)
		{
			dispatcher->AddPendingCloseSession(_sessionId);
			return;
		}

		FlushSendOwner(dispatcher);
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


}

         