#include "Pch.h"
#include "AuthService.h"
#include "SqliteDb.h"
#include "SQLiteStmt.h"
#include "Protocol.h"

#include <chrono>
#include <sqlite3.h>
#include <sodium.h>

AuthService::AuthService(SqliteDb *db)
    :   _db(db)
{
 
}

bool AuthService::Initialize()
{
    //  libsodium 초기화    
    if (sodium_init() < 0)
        return false;

    return true;
}

bool AuthService::RegisterUser(const std::string &username, const std::string &password, uint64 &userId, AuthFailReason &authFailReason)
{
    std::string pwHashString;
    if(HashPassword(password, pwHashString) == false)
    {
        authFailReason = AuthFailReason::FailedHashPassword;
        return false;
    }

    sqlite3* db = _db->GetDBHandle();
    if(db == nullptr)
    {
        authFailReason = AuthFailReason::DbError;
        return false;
    }

    if(BeginTransaction() == false)
    {
        authFailReason = AuthFailReason::DbError;
        return false;
    }

    SQLiteStmt stmt(db, "INSERT INTO users(username, hashed_password, created_at) VALUES(?, ?, ?);");
    if(stmt.BindText(1, username) == false || stmt.BindText(2, pwHashString) == false || stmt.BindInt64(3, NowSecond()) == false)
    {
        authFailReason = AuthFailReason::DbError;
        RollbackTransaction();
        return false;
    }

    if(stmt.StepDone() == false)
   {
        authFailReason = AuthFailReason::DbError;
        RollbackTransaction();
        return false;
    }

    userId = static_cast<uint64>(sqlite3_last_insert_rowid(db));

    if(CommitTransaction() == false)
    {
        authFailReason = AuthFailReason::DbError;
        RollbackTransaction();
        return false;
    }

    return true;
}

bool AuthService::Login(const std::string &username, const std::string &password, LoginResult &loginResult, AuthFailReason &authFailReason)
{
    sqlite3* db = _db->GetDBHandle();
    if(db == nullptr)
    {
        authFailReason = AuthFailReason::DbError;
        return false;
    }

    uint64 userId = 0;
    std::string pwHashString;

    {
        SQLiteStmt stmt(db,"SELECT user_id, hashed_password FROM users WHERE username = ?;");
        if(stmt.BindText(1, username) == false)
        {
            authFailReason = AuthFailReason::DbError;
            return false;
        }

        if(stmt.StepRow() == false)
        {
            authFailReason = AuthFailReason::UserNotFound;
            return false;
        }

        userId = static_cast<uint64>(stmt.GetInt64(0));
        pwHashString = stmt.GetText(1);

        if(VerifyPassword(password, pwHashString) == false)
        {
            authFailReason = AuthFailReason::WrongPassword;
            return false;
        }
    }

    if(BeginTransaction() == false)
    {
        authFailReason = AuthFailReason::DbError;
        return false;
    }

    {
        SQLiteStmt stmt(db,"UPDATE sessions SET revoked = 1 WHERE user_id = ?;");
        if(stmt.BindInt64(1, static_cast<int64>(userId)) == false)
        {
            authFailReason = AuthFailReason::DbError;
            RollbackTransaction();
            return false;
        }
        
        if(stmt.StepDone() == false)
        {
            authFailReason = AuthFailReason::DbError;
            RollbackTransaction();
            return false;
        }
    }

    const std::string token = MakeRandomToken();
    const int64 now = NowSecond();
    const int64 expires = now + 7 * 24 * 60 * 60; // 7일

    {
        SQLiteStmt stmt(db,"INSERT INTO sessions(token, user_id, expires_at, last_seen_at, revoked) VALUES(?, ?, ?, ?, 0);");
        if(stmt.BindText(1, token) == false || stmt.BindInt64(2, static_cast<int64>(userId)) == false || stmt.BindInt64(3, expires) == false || stmt.BindInt64(4, now) == false)
        {
            authFailReason = AuthFailReason::DbError;
            RollbackTransaction();
            return false;
        }
        
        if(stmt.StepDone() == false)
        {
            authFailReason = AuthFailReason::UserNotFound;
            RollbackTransaction();
            return false;
        }
    }

    if(CommitTransaction() == false)
    {
        authFailReason = AuthFailReason::DbError;
        RollbackTransaction();
        return false;
    }

    loginResult.userId;
    loginResult.token = token;
    loginResult.expiresAt = expires;

    return true;
}

bool AuthService::Resume(const std::string &token, uint64 &userId, AuthFailReason &authFailReason)
{
    sqlite3* db = _db->GetDBHandle();
    if(db == nullptr)
    {
        authFailReason = AuthFailReason::DbError;
        return false;
    }

    const int64 now = NowSecond();

    int64 getUserId = 0;
    int64 expiresAt = 0;
    int32 revoked = 0;

    {
        SQLiteStmt stmt(db, "SELECT user_id, expires_at, revoked FROM sessions WHERE token = ?;");
        if(stmt.BindText(1, token) == false)
        {
            authFailReason = AuthFailReason::DbError;
            return false;
        }

        if(stmt.StepRow() == false)
        {
            authFailReason = AuthFailReason::TokenInvalid;
            return false;
        }

        getUserId = stmt.GetInt64(0);
        expiresAt = stmt.GetInt64(1);
        revoked = stmt.GetInt(2);
    }

    if (revoked != 0) 
    {
        authFailReason = AuthFailReason::TokenInvalid;
        return false;
    }

    if (expiresAt <= now) 
    {
        authFailReason = AuthFailReason::TokenExpired;
        return false;
    }

    {
        SQLiteStmt stmt(db, "UPDATE sessions SET last_seen_at = ? WHERE token = ?;");
        if(stmt.BindInt64(1, now) == false || stmt.BindText(2, token) == false)
        {
            authFailReason = AuthFailReason::DbError;
            return false;
        }
        
        if(stmt.StepDone() == false)
        {
            authFailReason = AuthFailReason::DbError;
            return false;
        }
    }

    userId = static_cast<uint64>(getUserId);

    return true;
}

int64 AuthService::NowSecond() const
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string AuthService::MakeRandomToken()
{
    //  난수 생성
    unsigned char random[32] = {0};
    randombytes_buf(random, sizeof(random));

    //  Base64 인코딩 ( 문자열 변환 )
    char base64[128] = {0};
    sodium_bin2base64( base64, sizeof(base64), random, sizeof(random), sodium_base64_VARIANT_URLSAFE_NO_PADDING);

    return std::string(base64);
}

bool AuthService::HashPassword(const std::string &password, std::string &pwHashString)
{
    char hashString[crypto_pwhash_STRBYTES];
    if(crypto_pwhash_str(hashString, password.c_str(), password.size(), crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE) != 0)
        return false;

    pwHashString = hashString;
    return true;
}

bool AuthService::VerifyPassword(const std::string &password, const std::string &hashString)
{
    return crypto_pwhash_str_verify(hashString.c_str(), password.c_str(), password.size() == 0);
}

bool AuthService::BeginTransaction()
{
    return _db->Exec("BEGIN IMMEDIATE;");
}

bool AuthService::CommitTransaction()
{
    return _db->Exec("COMMIT;");
}

bool AuthService::RollbackTransaction()
{
    return _db->Exec("ROLLBACK;");
}
