#include "Pch.hpp"
#include "SessionManager.hpp"
#include "Session.hpp"

namespace servercore
{
    SessionManager::SessionManager()
    {
        Clear();
    }

    void SessionManager::Clear()
    {
        //  TODO
        //	Session Clear
		std::vector<std::shared_ptr<Session>> sessions;
		{
			WriteLockGuard lock(_lock);
			for (const auto& session : _sessions)
				sessions.push_back(session.second);
		}

		if (sessions.empty() == false)
		{
			std::cout << sessions.size() << " Active sessions to disconnect..." << std::endl;
			for (const auto& session : sessions)
			{
				if (session)
					session->Disconnect();
			}

			std::cout << "All sessions have completed disconnect..." << std::endl;
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

    void SessionManager::RequestRemoveSessionEvent(uint64 sessionId)
    {
        {
            //  TODO
            WriteLockGuard lockGuard(_lock);
            _removeSessionQueue.push(sessionId);
        }
    }

    void SessionManager::RemoveSession()
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