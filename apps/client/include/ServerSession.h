#pragma once

#include "network/Session.h"


class ServerSession : public network::Session
{
public:
    virtual void OnConnected() override
    {
       
    }

    virtual void OnDisconnected() override
    {

    }

    virtual void OnRecv(BYTE* buffer, int32 numOfBytes) override
    {
        
    }

    virtual void OnSend() override
    {

    }

private:
};