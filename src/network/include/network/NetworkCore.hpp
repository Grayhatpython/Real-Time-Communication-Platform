#pragma once    
#include "NetworkAddress.hpp"

namespace network
{
    class Session;
    class Acceptor;
    class GlobalContext;
    class ISessionRegistry;
    class INetworkDispatcher;
    class NetworkCore : public std::enable_shared_from_this<NetworkCore>
    {
    public:
        explicit NetworkCore(ISessionRegistry*  sessionRegistry);
        virtual ~NetworkCore();

        NetworkCore(const NetworkCore&) = delete;
        NetworkCore& operator=(const NetworkCore&) = delete;

    public:
        virtual void Stop();
    
    protected:
        void NetworkDispatch();

    private:
        void Initialize();

    protected:
        std::shared_ptr<INetworkDispatcher>             _networkDispatcher;
        ISessionRegistry*                               _sessionRegistry = nullptr;

        std::thread                                     _dispatchThread;

        std::mutex                                      _mutex;
        std::condition_variable                         _cv;
        std::atomic<bool>                               _isRunning{false};
        std::vector<std::thread>                        _workerThreads;
        int32                                           _workerThreadCount = 0;

    };

    class Server : public NetworkCore
    {
    public:
       explicit Server(ISessionRegistry*  sessionRegistry);
       virtual ~Server() override;

    public:
        bool Start(uint16 port);
        virtual void Stop() override;

    private:
        NetworkAddress						_listenNetworkAddress;
		std::shared_ptr<Acceptor>			_acceptor;
		uint16								_port = 0;
    };

    class Client : public NetworkCore
    {
    public:
       explicit Client(ISessionRegistry*  sessionRegistry);
       virtual ~Client() override;

    public:
        bool Connect(NetworkAddress& targetAddress, int32 connectionCount = 1);
        virtual void Stop() override;

    private:

    };
}