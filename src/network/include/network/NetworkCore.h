#pragma once    
#include "NetworkAddress.h"

namespace network
{
    class Session;
    class Acceptor;
    class GlobalContext;
    class SessionRegistry;
    class EpollDispatcher;
    class Server 
    {
    public:
       explicit Server(SessionRegistry*  sessionRegistry, uint16 port, int32 dispatchThreadCount);
       ~Server();

    public:
        bool    Initialize();
        void    Stop();

        std::function<void(std::stop_token)> MakeDispatchTask(int idx);
        std::function<void(std::stop_token)> MakeAcceptTask();

    public:
        int32       GetDispatchThreadCount() const { return _dispatchThreadCount; }
        
    private:
        SessionRegistry*                                _sessionRegistry = nullptr;
        std::vector<std::shared_ptr<EpollDispatcher>>   _dispatchers;
        int32                                           _dispatchThreadCount = 1;

		std::shared_ptr<Acceptor>			_acceptor;
		uint16								_port = 0;
    };

    class Client 
    {
    public:
       explicit Client(SessionRegistry*  sessionRegistry, int32 dispatchThreadCount = 1);
       ~Client();

    public:
        void    Connect(NetworkAddress& targetAddress, int32 connectionCount = 1);
        bool    Initialize();
        void    Stop();

    public:
        std::function<void(std::stop_token)> MakeDispatchTask();
        
    public:
        int32       GetDispatchThreadCount() const { return _dispatchThreadCount; }

    private:
        SessionRegistry*                                _sessionRegistry = nullptr;
        std::vector<std::shared_ptr<EpollDispatcher>>   _dispatchers;
        int32                                           _dispatchThreadCount = 1;
    };
}