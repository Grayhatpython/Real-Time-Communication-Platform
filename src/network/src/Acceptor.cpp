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

	bool Acceptor::Initialize(uint16 port, int32 backlog)
	{
		if (_listenSocketFd != INVALID_SOCKET_FD_VALUE)
			return false;

		//	Overlapped Socket
		_listenSocketFd = NetworkUtils::CreateSocketFd(true);
		if (_listenSocketFd == INVALID_SOCKET_FD_VALUE)
		{
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

		_epollFd = ::epoll_create1(EPOLL_CLOEXEC);
		if (_epollFd == INVALID_EPOLL_FD_VALUE)
			return false;

		_wakeupFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
		if (_wakeupFd == INVALID_EVENT_FD_VALUE)
			return false;

		epoll_event listen{};
		listen.events = EPOLLIN;
		listen.data.fd = _listenSocketFd;
		if (::epoll_ctl(_epollFd, EPOLL_CTL_ADD, _listenSocketFd, &listen) == RESULT_ERROR)
			return false;

		epoll_event wakeup{};
		wakeup.events = EPOLLIN;
		wakeup.data.fd = _wakeupFd;
		if (::epoll_ctl(_epollFd, EPOLL_CTL_ADD, _wakeupFd, &wakeup) == RESULT_ERROR)
			return false;

		_port = port;
		return true;
	}

	void Acceptor::Stop()
	{
		_running.store(false, std::memory_order_release);

		uint64_t one = 1;
		::write(_wakeupFd, &one, sizeof(one));

		EN_LOG_INFO("Acceptor Stopped");
	}

	void Acceptor::Run(std::stop_token st)
	{
		while (_running.load(std::memory_order_acquire) && st.stop_requested() == false)
		{
			int32 numOfEvents = ::epoll_wait(_epollFd, _epollEvents.data(), static_cast<int32>(_epollEvents.size()), INFINITY);

			if (numOfEvents == -1)
			{
				if (errno == EINTR)
					continue;

				break;
			}

			if (numOfEvents > 0)
			{
				for (int32 i = 0; i < numOfEvents; i++)
				{
			  		if(_epollEvents[i].data.fd == _wakeupFd)
					{
						for (;;)
						{
							uint64_t v;
							ssize_t r = ::read(_wakeupFd, &v, sizeof(v));
							if (r == sizeof(v))
								continue;
							if (r == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
								break;
							if (r == -1 && errno == EINTR)
								continue;
							break;
						}
						continue;
					}


					if(_epollEvents[i].data.fd == _listenSocketFd)
					{
						//	Event 처리동안 ref 늘려서 혹시나 하는상황 대비
						auto acceptEvent = engine::cnew<AcceptEvent>();
						acceptEvent->SetOwner(shared_from_this());

						for (;;)
						{
							sockaddr_in clientAddr{};
							socklen_t clientAddrLen = sizeof(clientAddr);

							int clientSocketFd = ::accept4(_listenSocketFd, (sockaddr *)&clientAddr, &clientAddrLen, SOCK_CLOEXEC | SOCK_NONBLOCK);
							if (clientSocketFd == INVALID_SOCKET_FD_VALUE)
							{
								if (errno == EAGAIN || errno == EWOULDBLOCK)
									break;
								if (errno == EINTR)
									continue;
								break;
							}

							int shardId = RandomShardId();

							_sessionRegistry->PostToShard(shardId, [this, shardId, clientSocketFd](SessionRegistry::Shard &shard, EpollDispatcher &dispatcher) {

								auto newSession = _sessionRegistry->CreateSession();
								if (newSession == nullptr) 
								{ 
									::close(clientSocketFd); 
									return; 
								}

								uint64 sessionId = shard.AllocateSessionId(uint16_t(shardId));
								newSession->SetSocketFd(clientSocketFd);
								newSession->SetSessionId(sessionId);

								// fd를 오너 dispatcher의 epoll에 등록
								if (dispatcher.Register(newSession) == false) 
								{
									dispatcher.UnRegister(newSession);
									::close(clientSocketFd);
									return;
								}

								if(NetworkUtils::SetTcpNoDelay(clientSocketFd, true) == false)
								{
									dispatcher.UnRegister(newSession);
									::close(clientSocketFd);
									return;
								}

								shard.sessions.emplace(sessionId, std::move(newSession)); });
						}

						if (acceptEvent)
						{
							engine::cdelete(acceptEvent);
							acceptEvent = nullptr;
						}
					}
				}
			}
		}

		Close();
	}

	void Acceptor::Close()
	{
		if (_wakeupFd != INVALID_EVENT_FD_VALUE)
		{
			::close(_wakeupFd);
			_wakeupFd = INVALID_EVENT_FD_VALUE;
		}
		if (_listenSocketFd != INVALID_SOCKET_FD_VALUE)
		{
			::close(_listenSocketFd);
			_listenSocketFd = INVALID_SOCKET_FD_VALUE;
		}
		if (_epollFd != INVALID_EPOLL_FD_VALUE)
		{
			::close(_epollFd);
			_epollFd = INVALID_EPOLL_FD_VALUE;
		}
	}

	void Acceptor::Dispatch(NetworkEvent *networkEvent)
	{
		
	}

	int32 Acceptor::RandomShardId()
	{
		return ((_autoIncrementShardId++) % _sessionRegistry->GetShardCount());
	}
}