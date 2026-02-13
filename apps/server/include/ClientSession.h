#pragma once

#include "network/Session.h"


class ClientSession final : public network::Session
{
public:
    virtual void OnConnected() override;
    virtual void OnDisconnected() override;
    virtual void OnRecv(BYTE* buffer, int32 numOfBytes) override;
    virtual void OnSend() override;
    
private:

};