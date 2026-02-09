#include "network/NetworkPch.h"
#include "network/NetworkCore.h"
#include "network/Session.h"
#include "network/Acceptor.h"
#include "network/NetworkUtils.h"
#include "network/NetworkDispatcher.h"
#include "network/SessionRegistry.h"
#include "network/SendBufferPool.h"
#include "network/NetworkCore.h"

#include "engine/Logger.h"

namespace network
{
    Server::Server(SessionRegistry*  sessionRegistry, uint16 port, int32 dispatchThreadCount)
        : _sessionRegistry(sessionRegistry), _dispatchThreadCount(dispatchThreadCount), _acceptor(std::make_shared<Acceptor>()), _port(port)
    {

    }

    Server::~Server()
    {
        
    }

    bool Server::Initialize()
    {
        _dispatchers.clear();
        _dispatchers.reserve(_dispatchThreadCount);

        for (int i = 0; i < _dispatchThreadCount; ++i) 
        {
            auto dispatcher = std::make_shared<EpollDispatcher>();
            dispatcher->SetSessionRegistry(_sessionRegistry);
            dispatcher->SetShardId(i);
            
            if (dispatcher->Initialize() == false) 
            {
                return false;
            }

            _dispatchers.emplace_back(std::move(dispatcher));
        }

        if(_acceptor->Initialize(_port) == false)
            return false;

        _acceptor->SetSessionRegistry(_sessionRegistry);
        
        return true;
    }

    void Server::Stop()
    {
        if(_acceptor)
        {
            _acceptor->Stop();
        }

        for (auto& dispatcher : _dispatchers)
        {
            dispatcher->Stop();
        }

        network::SendBufferArena::SendBufferPoolClear();
    }


    std::function<void(std::stop_token)> Server::MakeDispatchTask(int idx)
    {
        return [this, idx](std::stop_token st) {
            _dispatchers[idx]->Run(st);
        };
    }

    std::function<void(std::stop_token)> Server::MakeAcceptTask()
    {
        return [this](std::stop_token st) {
            _acceptor->Run(st);
        };
    }

    Client::Client(SessionRegistry*  sessionRegistry , int32 dispatchThreadCount)
        : _sessionRegistry(sessionRegistry), _dispatchThreadCount(dispatchThreadCount)
    {

    }

    Client::~Client()
    {

    }

    void Client::Connect(NetworkAddress& targetAddress, int32 connectionCount)
    {
        _sessionRegistry->ConnectAsync(0, targetAddress);
    }

    bool Client::Initialize()
    {
        _dispatchers.clear();
        _dispatchers.reserve(_dispatchThreadCount);

        for (int i = 0; i < _dispatchThreadCount; ++i)
        {
            auto dispatcher = std::make_shared<EpollDispatcher>();
            dispatcher->SetSessionRegistry(_sessionRegistry);
            dispatcher->SetShardId(i);
            
            if (dispatcher->Initialize() == false) 
            {
                return false;
            }

            _dispatchers.emplace_back(std::move(dispatcher));
        }
        return true;
    }

    void Client::Stop()
    {
        for (auto& dispatcher : _dispatchers)
        {
            dispatcher->Stop();
        }

        network::SendBufferArena::SendBufferPoolClear();
    }

    std::function<void(std::stop_token)> Client::MakeDispatchTask()
    {
        return [this](std::stop_token st) {
            _dispatchers[0]->Run(st);
        };
    }

}