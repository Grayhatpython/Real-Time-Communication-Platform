#include "Pch.hpp"
#include "SessionManager.hpp"
#include "Session.hpp"
#include "NetworkDispatcher.hpp"

namespace servercore
{
    SessionManager::SessionManager()
    {

    }

    void SessionManager::Clear()
    {
        //  TODO
        //	Session Clear
        //  여기 문제네
		std::vector<std::shared_ptr<Session>> sessions;
		{
			WriteLockGuard lock(_lock);
			for (const auto& session : _sessions)
				sessions.push_back(session.second);
		}

		if (sessions.empty() == false)
		{
            NC_LOG_INFO("{} Active sessions to disconnect", sessions.size());

			for (const auto& session : sessions)
			{
				if (session)
					session->Disconnect();
			}

            NC_LOG_INFO("All sessions have completed disconnected");
		}	

        sessions.clear();
    }

    void SessionManager::AddSession(std::shared_ptr<Session> session)
    {
        if(session == nullptr)
            return;

        WriteLockGuard lockGuard(_lock);
        _sessions.insert(std::make_pair(session->GetSessionId(), session));
    }

    void SessionManager::PushRemoveSessionEvent(uint64 sessionId)
    {
        {
            //  TODO
            WriteLockGuard lockGuard(_lock);
            _removeSessionQueue.push(sessionId);
        }
    }

    void SessionManager::ProcessRemoveSessionEvent()
    {
        WriteLockGuard lockGuard(_lock);
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
        WriteLockGuard lockGuard(_lock);
        _sessions.erase(sessionId);
    }

    void SessionManager::GetSessions(std::vector<std::shared_ptr<Session>>& sessions)
    {
        WriteLockGuard lockGuard(_lock);
        for(const auto& session : _sessions)
        {
            sessions.push_back(session.second);
        }
    }

    std::shared_ptr<Session> SessionManager::CreateSession()
    {
        auto session = _sessionFactory();
        if(session == nullptr)
            return nullptr;

        //  TODO
        return session;
    }

    uint64 SessionManager::GetSessionCount()
    {
        uint64 count = 0;
        {
            WriteLockGuard lockGuard(_lock);
            count = _sessions.size();
        }
        
        return count;
    }
}