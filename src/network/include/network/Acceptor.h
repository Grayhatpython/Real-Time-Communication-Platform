#pragma once

namespace network
{
    class AcceptEvent;
	class SessionRegistry;
	class Acceptor : public std::enable_shared_from_this<Acceptor>
	{
        static constexpr size_t S_DEFALUT_EPOLL_EVENT_SIZE = 64;
		static constexpr uint64_t kListenTag = 1;
		static constexpr uint64_t kWakeTag   = 2;
	
	public:
		Acceptor();
		~Acceptor();

	public:
		bool Initialize(uint16 port, int32 backlog = SOMAXCONN);
		void Stop();
     	void Run(std::stop_token st);

	public:
		SocketFd            GetSocketFd() const { return _listenSocketFd; } 

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