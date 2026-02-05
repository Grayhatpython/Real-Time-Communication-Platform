#pragma once
#include "NetworkInterface.hpp"

namespace network
{
    class AcceptEvent;
	class Acceptor : public EpollObject
	{
		friend class Server;
		
	public:
		Acceptor();
		virtual ~Acceptor() override;

	public:
		bool Start(uint16 port, int32 backlog = SOMAXCONN);
		void Stop();

	public:
		virtual NetworkObjectType   GetNetworkObjectType() override { return NetworkObjectType::Acceptor; }
		virtual SocketFd            GetSocketFd() const override { return _listenSocketFd; } 
		virtual void                Dispatch(INetworkEvent* networkEvent) override;

	private:
		void                        ProcessAccept(AcceptEvent* acceptEvent);

	public:
		void									SetNetworkDispatcher(std::shared_ptr<INetworkDispatcher> networkDispatcher) { _networkDispatcher = networkDispatcher; }
		std::shared_ptr<INetworkDispatcher>		GetNetworkDispatcher() { return _networkDispatcher; }

	private:
        SocketFd								_listenSocketFd = INVALID_SOCKET_FD_VALUE;
		std::shared_ptr<INetworkDispatcher> 	_networkDispatcher;
	};
}