#pragma once

#include "network/Session.h"
#include "World.h"

class ClientSession : public network::Session
{
public:
    ~ClientSession()
    {
        if(_world)
        {
            delete _world;
            _world = nullptr;
        }
    }
    
public:
    virtual void OnConnected() override
    {
        auto clientSession = std::static_pointer_cast<ClientSession>(shared_from_this());
        _world->Enter(clientSession);
    }

    virtual void OnDisconnected() override
    {
        auto clientSession = std::static_pointer_cast<ClientSession>(shared_from_this());
        _world->Leave(clientSession);   
    }

    virtual void OnRecv(BYTE* buffer, int32 numOfBytes) override
    {
      
    }

    virtual void OnSend() override
    {

    }

private:
    //  TEMP
    World* _world = nullptr;
};