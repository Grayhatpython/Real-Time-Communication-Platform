#include "Pch.hpp"
#include "Acceptor.hpp"
#include "NetworkCore.hpp"
#include "NetworkUtils.hpp"
#include "NetworkEvent.hpp"
#include "NetworkDispatcher.hpp"

namespace servercore
{
	Acceptor::Acceptor()
	{

	}

	Acceptor::~Acceptor() 
	{

	}

	bool Acceptor::Start(uint16 port, int32 backlog = SOMAXCONN)
	{
		if (_listenSocketFd == INVALID_SOCKET_FD_VALUE)
		{
			return false;
		}

		//	Overlapped Socket
		_listenSocketFd = NetworkUtils::CreateSocketFd(true);
		if (_listenSocketFd == INVALID_SOCKET_FD_VALUE)
		{
			return false;
		}

		if (_networkDispatcher->Register(shared_from_this()) == false)
		{
			NetworkUtils::CreateSocketFd(_listenSocketFd);
			return false;
		}

		if (NetworkUtils::SetReuseAddress(_listenSocketFd, true) == false)
		{
			NetworkUtils::CreateSocketFd(_listenSocketFd);
			return false;
		}

		if (NetworkUtils::Bind(_listenSocketFd, port) == false)
		{
			NetworkUtils::CreateSocketFd(_listenSocketFd);
			return false;
		}

		if (NetworkUtils::Listen(_listenSocketFd, backlog) == false)
		{
			NetworkUtils::CreateSocketFd(_listenSocketFd);
			return false;
		}

		return true;
	}

	void Acceptor::Stop()
	{
        //  TODO
		auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
		if(epollDispatcher->UnRegister(shared_from_this()) == false)
			;	//	???

		NetworkUtils::CloseSocketFd(_listenSocketFd);
	}

	void Acceptor::Dispatch(INetworkEvent* networkEvent, bool succeeded, int32 errorCode)
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
			auto newSession = INetworkCore::CreateSession();
			assert(newSession);

			if(NetworkUtils::SetReuseAddress(clientSocketFd, true) == false)
			{
				//	TODO
				return;
			}

			newSession->SetSocket(clientSocketFd);
			newSession->SetRemoteAddress(remoteAddress);
			
			if (_networkDispatcher->Register(std::static_pointer_cast<INetworkObject>(newSession)) == false)
			{
				//	TODO
				return;
			}

			_serverCore->AddSession(newSession);
			newSession->ProcessConnect();
		}

		if(acceptEvent)
		{
			cdelete(acceptEvent);
			acceptEvent = nullptr;
		}
	}
}