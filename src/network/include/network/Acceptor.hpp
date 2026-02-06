#pragma once
#include "NetworkInterface.hpp"

namespace network
{
    class AcceptEvent;
	class ISessionRegistry;
	class Acceptor : public IEpollObject
	{
		
	public:
		Acceptor();
		virtual ~Acceptor() override;

	public:
		bool Start(uint16 port, int32 backlog = SOMAXCONN);
		void Stop();

	public:
		virtual NetworkObjectType   GetNetworkObjectType() override { return NetworkObjectType::Acceptor; }
		virtual SocketFd            GetSocketFd() const override { return _listenSocketFd; } 
		virtual void                Dispatch(NetworkEvent* networkEvent) override;

	private:
		void                        ProcessAccept(AcceptEvent* acceptEvent);

	public:
		void									SetNetworkDispatcher(std::shared_ptr<INetworkDispatcher> networkDispatcher) { _networkDispatcher = networkDispatcher; }
		std::shared_ptr<INetworkDispatcher>		GetNetworkDispatcher() { return _networkDispatcher; }

		void									SetSessionRegistry(ISessionRegistry*  sessionRegistry) { _sessionRegistry = sessionRegistry; }
		ISessionRegistry*  						GetSessionRegistry() { return _sessionRegistry; }

	private:
        SocketFd								_listenSocketFd = INVALID_SOCKET_FD_VALUE;
		std::shared_ptr<INetworkDispatcher> 	_networkDispatcher;
		ISessionRegistry*  						_sessionRegistry = nullptr;
	};
}