#include "Pch.hpp"
#include "SessionManager.hpp"

SessionManager::~SessionManager()
{
    std::cout << "~SessionManager" << std::endl;
}

void SessionManager::Clear()
{
    //  TODO
    //	Session Clear
    //  여기 문제네
    std::vector<std::shared_ptr<network::Session>> sessions;
    {
        engine::WriteLockGuard lock(_lock);
        for (const auto& session : _sessions)
            sessions.push_back(session.second);
    }

    if (sessions.empty() == false)
    {
        EN_LOG_INFO("{} Active sessions to disconnect", sessions.size());

        for (const auto& session : sessions)
        {
            if (session)
                session->Disconnect();
        }

        EN_LOG_INFO("All sessions have completed disconnected");
    }	

    sessions.clear();
}

void SessionManager::AddSession(std::shared_ptr<network::Session> session)
{
    if(session == nullptr)
        return;

    engine::WriteLockGuard lockGuard(_lock);
    _sessions.insert(std::make_pair(session->GetSessionId(), session));
}

void SessionManager::PushRemoveSessionEvent(uint64 sessionId)
{
    {
        //  TODO
        engine::WriteLockGuard lockGuard(_lock);
        _removeSessionQueue.push(sessionId);
    }
}

void SessionManager::ProcessRemoveSessionEvent()
{
    engine::WriteLockGuard lockGuard(_lock);
    std::queue<uint64> removeSessions;
    {
        removeSessions.swap(_removeSessionQueue);
    }

    while (removeSessions.empty() == false)
    {
        _sessions.erase(removeSessions.front());
        removeSessions.pop();
    }
}

void SessionManager::AbortSession(uint64 sessionId)
{
    engine::WriteLockGuard lockGuard(_lock);
    _sessions.erase(sessionId);
}


std::shared_ptr<network::Session> SessionManager::CreateSession()
{
    auto session = _sessionFactory();
    if(session == nullptr)
        return nullptr;

    //  TODO
    return session;
}

