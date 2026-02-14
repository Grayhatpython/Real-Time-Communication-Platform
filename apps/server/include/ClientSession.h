#pragma once

#include "network/Session.h"


class ClientSession final : public network::Session
{
public:
    virtual void OnConnected() override;
    virtual void OnDisconnected() override;
    virtual void OnRecv(BYTE* buffer, int32 numOfBytes) override;
    virtual void OnSend() override;
    
public:
    void    SetUserId(uint64 userId) { _userId = userId; }
    uint64  GetUserId() const { return _userId; }

    bool    IsAuthed() const { return _userId != 0; }


private:
    uint64  _userId = 0;
};