#include "Pch.h"
#include "AuthService.h"
#include "SqliteDb.h"
#include "SQLiteStmt.h"

#include <chrono>
#include <sqlite3.h>
#include <sodium.h>

AuthService::AuthService(SqliteDb *db)
    :   _db(db)
{
    //  libsodium 초기화    
    sodium_init();
}

bool AuthService::CreateUser(const std::string &username, const std::string &password, uint64 &userId, AuthFailReason &authFailReason)
{
    std::string pwHashString;
    if(HashPassword(password, pwHashString) == false)
    {
        authFailReason = AuthFailReason::FailedHashPassword;
        return false;
    }

    if(BeginTransaction() == false)
    {
        authFailReason = AuthFailReason::DbError;
        return false;
    }

    sqlite3* db = _db->GetDBHandle();
    if(db == nullptr)
    {
        authFailReason = AuthFailReason::DbError;
        RollbackTransaction();
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
    return false;
}

bool AuthService::Reconnect(const std::string &token, uint64 &userId, AuthFailReason &authFailReason)
{
    return false;
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
