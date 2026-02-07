#include "network/NetworkPch.hpp"
#include "network/NetworkCore.hpp"
#include "network/Session.hpp"
#include "network/Acceptor.hpp"
#include "network/NetworkUtils.hpp"
#include "network/NetworkDispatcher.hpp"
#include "network/SessionRegistry.hpp"
#include "network/SendBufferPool.hpp"
#include "network/NetworkCore.hpp"

namespace network
{
    NetworkCore::NetworkCore(ISessionRegistry*  sessionRegistry)
        : _sessionRegistry(sessionRegistry)
    {
        Initialize();
    }

    NetworkCore::~NetworkCore()
    {

    }

    void NetworkCore::Stop()
    {
  
    }

    std::function<void(std::stop_token)> NetworkCore::MakeDispatchTask()
    {
        return [this](std::stop_token st) {
            auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
            
            if(epollDispatcher)
            {
                epollDispatcher->Run(st);
            }
        };
    }

    void NetworkCore::StopDispatchTask()
    {
        auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
        
        if(epollDispatcher)
        {
            epollDispatcher->Stop();
        }
    }

    void NetworkCore::Initialize()
    {
        NetworkUtils::Initialize();

        {
            _networkDispatcher = std::make_shared<EpollDispatcher>();
            auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
            
            if(epollDispatcher)
            {
                epollDispatcher->Initialize();
            }
        }
    }

    Server::Server(ISessionRegistry*  sessionRegistry)
        : NetworkCore(sessionRegistry), _acceptor(std::make_shared<Acceptor>())
    {

    }

    Server::~Server()
    {
        
    }

    bool Server::Start(uint16 port)
    {
        if(_acceptor == nullptr)
            return false;

        if(_networkDispatcher == nullptr)
            return false;

        if(_sessionRegistry == nullptr)
            return false;

        _acceptor->SetNetworkDispatcher(_networkDispatcher);
        _acceptor->SetSessionRegistry(_sessionRegistry);
        
        auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
        epollDispatcher->SetSessionRegistry(_sessionRegistry);

        if(_acceptor->Start(port) == false)
        {
            return false;
        }
        
        _port = port;
        _isRunning.store(true, std::memory_order_release);
    
        EN_LOG_INFO("Server port:{} Started", _port);

        return true;
    }

    void Server::StopDispatchTask()
    {
        if(_isRunning.load(std::memory_order_acquire) == false)
            return;
        
        EN_LOG_INFO("Server Stopping");
            
        if(_acceptor)
            _acceptor->Stop();

        NetworkCore::StopDispatchTask();

        EN_LOG_INFO("Server Stopped");
    }
    
    Client::Client(ISessionRegistry*  sessionRegistry)
        : NetworkCore(sessionRegistry)
    {

    }

    Client::~Client()
    {

    }

    bool Client::Connect(NetworkAddress& targetAddress, int32 connectionCount)
    {
        if(_networkDispatcher == nullptr)
            return false;

        if(_sessionRegistry == nullptr)
            return false;
            
        EN_LOG_INFO("Client target Address:{} port:{} Connecting", targetAddress.GetIpStringAddress(), targetAddress.GetPort());
            
        auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
        epollDispatcher->SetSessionRegistry(_sessionRegistry);

        std::vector<uint64> connectedSessions;  

        for (int i = 0; i < connectionCount; ++i)
        {
            auto session = _sessionRegistry->CreateSession();
     
            _sessionRegistry->AddSession(session);
            connectedSessions.push_back(session->GetSessionId());

            session->SetNetworkDispatcher(_networkDispatcher);

            if (session->Connect(targetAddress) == false)
            {
                // 1) 이번 세션은 즉시 파기(정상 진입 전)
                _sessionRegistry->AbortSession(session->GetSessionId());

                // 2) 이미 만들어진 다른 세션들 정리
                for (auto id : connectedSessions)
                {
                    if (id == session->GetSessionId()) 
                        continue;

                    epollDispatcher->PostRemoveSessionEvent(id);
                }

                return false;
            }
        }

        _isRunning.store(true, std::memory_order_release);

        return true;
    }

    void Client::StopDispatchTask()
    {
        if(_isRunning.load(std::memory_order_acquire) == false)
            return;

        EN_LOG_INFO("Client Stopping");

        NetworkCore::StopDispatchTask();

        EN_LOG_INFO("Client Stopped");
    }
    
}