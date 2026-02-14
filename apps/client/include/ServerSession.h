#pragma once

#include "network/Session.h"


class ServerSession : public network::Session
{
public:
    virtual void OnConnected() override;
    virtual void OnDisconnected() override;
    virtual void OnRecv(BYTE* buffer, int32 numOfBytes) override;
    virtual void OnSend() override;

public:

public:
    bool            IsAuthed() const { return _authed == true; }
    void            SetAuthed(uint64_t userId, const std::string& token) { _userId = userId; _token = token; };
    uint64          GetUserId() const { return _userId; }
    std::string     GetToken() const { return _token; }

private:
    std::vector<std::string> _logs;
    std::string             _token;
    uint64_t                _userId{0};
    bool                    _authed{false};
};