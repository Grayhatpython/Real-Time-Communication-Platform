#pragma once

#include "network/Session.h"

class ClientSession : public network::Session
{
public:
    ClientSession()
    {

    }

    virtual ~ClientSession()
    {
        
    }
    
public:
    virtual void OnConnected() override
    {
        auto clientSession = std::static_pointer_cast<ClientSession>(shared_from_this());
    }

    virtual void OnDisconnected() override
    {
        auto clientSession = std::static_pointer_cast<ClientSession>(shared_from_this());
    }

    virtual void OnRecv(BYTE* buffer, int32 numOfBytes) override
    {
      
    }

    virtual void OnSend() override
    {

    }

private:
};