#pragma once    

namespace servercore
{
    class Session;
    class Acceptor;
    class GlobalContext;
    class INetworkDispatcher;

    class INetworkCore : public std::enable_shared_from_this<INetworkCore>
    {
    public:

    public:
        static std::shared_ptr<Session>                 CreateSession();
        
    protected:
        GlobalContext*                                  _globalContext = nullptr;
        std::shared_ptr<INetworkDispatcher>             _networkDispatcher;

        std::mutex                                      _mutex;
        std::condition_variable                         _cv;
        std::atomic<bool>                               _isRunning{false};
        std::vector<std::thread>                        _workerThreads;
        int32                                           _workerThreadCount = 0;

        std::unordered_set<std::shared_ptr<Session>>    _sessions;
        std::function<std::shared_ptr<Session>(void)>   _sessionFactory;
    };

    class Server : public INetworkCore
    {
    public:

    private:
    };

    class Client : public INetworkCore
    {
    public:

    private:
    };
}