#pragma once
#include "NetworkInterface.hpp"

namespace network
{
    class AcceptEvent;
	class SessionRegistry;
	class Acceptor : public IEpollObject
	{
        static constexpr size_t S_DEFALUT_EPOLL_EVENT_SIZE = 64;
		static constexpr uint64_t kListenTag = 1;
		static constexpr uint64_t kWakeTag   = 2;
	
	public:
		Acceptor();
		virtual ~Acceptor() override;

	public:
		bool Initialize(uint16 port, int32 backlog = SOMAXCONN);
		void Stop();
     	void Run(std::stop_token st);

	public:
		virtual NetworkObjectType   GetNetworkObjectType() override { return NetworkObjectType::Acceptor; }
		virtual SocketFd            GetSocketFd() const override { return _listenSocketFd; } 
		virtual void                Dispatch(NetworkEvent* networkEvent) override;

	private:
		//	TEMP
		int32						RandomShardId();
		void 						Close();

	public:
		void									SetSessionRegistry(SessionRegistry*  sessionRegistry) { _sessionRegistry = sessionRegistry; }
		SessionRegistry*  						GetSessionRegistry() { return _sessionRegistry; }

	private:
		std::vector<struct epoll_event> 		_epollEvents;
		SessionRegistry*  						_sessionRegistry = nullptr;
		
        SocketFd								_listenSocketFd = INVALID_SOCKET_FD_VALUE;
		EpollFd									_epollFd = INVALID_EPOLL_FD_VALUE;
		EventFd									_wakeupFd = INVALID_EVENT_FD_VALUE;

		std::atomic<bool>						_running{true};
        int32                           		_autoIncrementShardId = 0;
		uint16									_port = 0;
	};
}