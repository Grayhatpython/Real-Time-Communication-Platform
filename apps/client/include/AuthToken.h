#pragma once

#include "engine/CommonType.h"

struct AuthToken
{
    uint64      userId = 0;
    int64       last_seen_at = 0;   // unix seconds
    int64       expires_at = 0;     // unix seconds
    std::string token;
};

class AuthTokenStore
{
public:
    explicit AuthTokenStore(std::string path);

public:
    bool Load(AuthToken& authToken);
    bool Save(const AuthToken& authToken);
    bool Clear();

public:
    const std::string&  Path() const { return _path; }
    static int64        NowSeconds();

private:
    std::string         _path;
};