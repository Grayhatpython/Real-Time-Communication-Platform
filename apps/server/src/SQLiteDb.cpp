#include "Pch.h"
#include "SqliteDb.h"

#include <stdexcept>
#include <sqlite3.h>

#include "engine/Logger.h"

SqliteDb::~SqliteDb() 
{
    Close();
}

bool SqliteDb::Open(const std::string& filePath) 
{
    Close();

    // flags: READWRITE + CREATE (파일 없으면 생성)
    const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int32 ret = sqlite3_open_v2(filePath.c_str(), &_db, flags, nullptr);
    
    if (ret != SQLITE_OK) 
    {   
        return false;
    }

    // 결과코드 확장(디버깅 시 도움)
    sqlite3_extended_result_codes(_db, 1);

    // 락 경합 시 바로 실패하지 않도록 대기(밀리초)
    sqlite3_busy_timeout(_db, 2000);

    return true;
}

void SqliteDb::Close() 
{
    if (_db) 
    {
        sqlite3_close(_db);
        _db = nullptr;
    }
}

bool SqliteDb::Exec(const std::string& sql) 
{
    char* errMsg = nullptr;
    int32 ret = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (ret != SQLITE_OK) 
    {
        std::string msg = errMsg ? errMsg : "unknown sqlite error";

        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

