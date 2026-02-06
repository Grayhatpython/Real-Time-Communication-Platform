#include "network/NetworkPch.hpp"
#include "network/Acceptor.hpp"
#include "network/NetworkCore.hpp"
#include "network/NetworkUtils.hpp"
#include "network/NetworkEvent.hpp"
#include "network/NetworkDispatcher.hpp"
#include "network/Session.hpp"
#include "network/SessionRegistry.hpp"

namespace network
{
	Acceptor::Acceptor()
	{

	}

	Acceptor::~Acceptor() 
	{

	}

	bool Acceptor::Start(uint16 port, int32 backlog)
	{
		if (_listenSocketFd != INVALID_SOCKET_FD_VALUE)
			return false;

		//	Overlapped Socket
		_listenSocketFd = NetworkUtils::CreateSocketFd(true);
		if (_listenSocketFd == INVALID_SOCKET_FD_VALUE)
		{
			return false;
		}

		if (_networkDispatcher->Register(shared_from_this()) == false)
		{
			NetworkUtils::CloseSocketFd(_listenSocketFd);
			return false;
		}

		if (NetworkUtils::SetReuseAddress(_listenSocketFd, true) == false)
		{
			NetworkUtils::CloseSocketFd(_listenSocketFd);
			return false;
		}

		if (NetworkUtils::Bind(_listenSocketFd, port) == false)
		{
			NetworkUtils::CloseSocketFd(_listenSocketFd);
			return false;
		}

		if (NetworkUtils::Listen(_listenSocketFd, backlog) == false)
		{
			NetworkUtils::CloseSocketFd(_listenSocketFd);
			return false;
		}

		return true;
	}

	void Acceptor::Stop()
	{
        if(_networkDispatcher)
		{
			auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
			if(epollDispatcher)
				epollDispatcher->UnRegister(shared_from_this());
		}

		if(_listenSocketFd != INVALID_SOCKET_FD_VALUE)
		{
			NetworkUtils::CloseSocketFd(_listenSocketFd);
			_listenSocketFd = INVALID_SOCKET_FD_VALUE;
		}	
	}

	void Acceptor::Dispatch(NetworkEvent* networkEvent)
	{
		if(networkEvent->GetNetworkEventType() == NetworkEventType::Accept)
		{
			auto session = shared_from_this();
			networkEvent->SetOwner(session);

			auto acceptEvent = static_cast<AcceptEvent*>(networkEvent);
			ProcessAccept(acceptEvent);
		}
		else if(networkEvent->GetNetworkEventType() == NetworkEventType::Error)
		{
			
		}
		else
		{
			
		}
	}

	void Acceptor:: ProcessAccept(AcceptEvent* acceptEvent)
	{
		struct sockaddr_in address;
		socklen_t addrLen = sizeof(address);

		while(true)
		{
			SocketFd clientSocketFd = ::accept4(_listenSocketFd, (struct sockaddr*)&address, &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
			if(clientSocketFd == INVALID_SOCKET_FD_VALUE)
			{
				//	더 이상 연결 요청 없음
				if(errno == EAGAIN || errno == EWOULDBLOCK)
					break;
				//	시스템 콜 인터럽트, 재시도
				else if(errno == EINTR)
					continue;

				///	???	그 외 오류
				break;
			}

			NetworkAddress remoteAddress(address);

            //  TODO
			auto newSession = _sessionRegistry->CreateSession();
			assert(newSession);

			if(NetworkUtils::SetReuseAddress(clientSocketFd, true) == false)
			{
				//	TODO
				return;
			}

			if(NetworkUtils::SetTcpNoDelay(clientSocketFd, true) == false)
			{
				return;
			}

			newSession->SetSocketFd(clientSocketFd);
			newSession->SetRemoteAddress(remoteAddress);

			if (_networkDispatcher->Register(std::static_pointer_cast<INetworkObject>(newSession)) == false)
			{
				//	TODO
				return;
			}

			_sessionRegistry->AddSession(newSession);
			newSession->ProcessConnect();
		}

		if(acceptEvent)
		{
			engine::cdelete(acceptEvent);
			acceptEvent = nullptr;
		}
	}
}