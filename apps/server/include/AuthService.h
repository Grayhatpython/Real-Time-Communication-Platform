#pragma once    

#include "engine/CommonType.h"
enum class AuthFailReason : uint16
{
    None = 0,
    UserNotFound,
    FailedHashPassword,
    WrongPassword,
    TokenInvalid,
    TokenExpired,
    DbError,
};

struct LoginResult
{
    uint64 userId = 0;
    std::string token;   // base64url
    int64 expiresAt = 0; // unix sec
};

class SqliteDb;
class AuthService
{
public:
    explicit AuthService(SqliteDb* db);

    // 회원 생성(중복 username이면 false)
    bool CreateUser(const std::string& username, const std::string& password, uint64& userId, AuthFailReason& authFailReason);

    // 로그인: username/password 검증 후 token 발급
    bool Login(const std::string& username, const std::string& password, LoginResult& loginResult, AuthFailReason& authFailReason);

    // 재접속: token 검증 후 userId 반환
    bool Reconnect(const std::string& token, uint64& userId, AuthFailReason& authFailReason);

private:
    int64       NowSecond() const;
    std::string MakeRandomToken(); // 32 bytes random -> base64url
    
    bool HashPassword(const std::string& password, std::string& pwHashString);
    bool VerifyPassword(const std::string& password, const std::string& hashString);
    
    bool BeginTransaction();
    bool CommitTransaction();
    bool RollbackTransaction();

private:
    SqliteDb* _db = nullptr;
};