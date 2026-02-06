#pragma once 
#include "network/Session.hpp"
#include "network/SessionRegistry.hpp"

class SessionManager final : public network::ISessionRegistry 
{
public:
    ~SessionManager() override;

public:
    void                                    AddSession(std::shared_ptr<network::Session> session) override;
    std::shared_ptr<network::Session>       CreateSession() override;
    void                                    Clear() override;

    void                                    PushRemoveSessionEvent(uint64 sessionId) override;
    void                                    ProcessRemoveSessionEvent() override;
    void                                    AbortSession(uint64 sessionId) override;

public:
    void                                            SetSessionFactory(std::function<std::shared_ptr<network::Session>()> sessionFactory) { _sessionFactory = sessionFactory; };

private:
    engine::Lock                                                        _lock;
    std::unordered_map<uint64, std::shared_ptr<network::Session>>       _sessions;
    std::queue<uint64>                                                  _removeSessionQueue;
    std::function<std::shared_ptr<network::Session>(void)>              _sessionFactory;
    uint64                                                              _sessionCount = 0;
};
