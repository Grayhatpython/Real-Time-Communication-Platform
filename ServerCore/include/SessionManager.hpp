#pragma once 

namespace servercore
{
    class Session;
    class SessionManager
    {
    public:
        explicit SessionManager();
        void Clear();

    public:
        void                        AddSession(std::shared_ptr<Session> session);
        std::shared_ptr<Session>    CreateSession();

        void                        RequestRemoveSessionEvent(uint64 sessionId);
        void                        RemoveSession();

    public:
	    void                                            SetSessionFactory(std::function<std::shared_ptr<Session>()> sessionFactory) { _sessionFactory = sessionFactory; }
        std::function<std::shared_ptr<Session>(void)>   GetSessionFactory() { return _sessionFactory; }
        uint64                                          GetSessionCount();

    private:
        Lock                                                    _lock;
        std::unordered_map<uint64, std::shared_ptr<Session>>    _sessions;
        std::queue<uint64>                                      _removeSessionQueue;
        std::function<std::shared_ptr<Session>(void)>           _sessionFactory;
        uint64                                                  _sessionCount = 0;
    };
}