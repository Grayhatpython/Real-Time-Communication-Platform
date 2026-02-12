#pragma once

#include "engine/CommonType.h"

class sqlite3;
class sqlite3_stmt;
class SQLiteStmt
{
public:
     SQLiteStmt(sqlite3* db, const std::string& sql);
    ~SQLiteStmt();

public:
    //  Bind 함수들은 인덱스가 1부터 시작하고, Get(Column) 함수들은 0부터 시작
    bool BindText(int32 index, const std::string& text);
    bool BindInt64(int32 index, int64 value);
    bool BindInt(int32 index, int32 value);
    bool BindDouble(int32 index, double value);
    bool BindBlob(int32 index, const void* data, int32 size);

    int32       GetInt(int32 columnIndex);
    int64       GetInt64(int32 columnIndex);
    double      GetDouble(int32 columnIndex);
    std::string GetText(int32 columnIndex);
    const void* GetBlob(int32 columnIndex, int32& size);

    bool StepRow();
    bool StepDone();

public:
    sqlite3_stmt*   GetStmt() const { return _stmt; }
    bool            IsPrepared() const { return _isPrepared && _stmt != nullptr; }

private:
    sqlite3*        _db = nullptr;
    sqlite3_stmt*   _stmt = nullptr;
    bool            _isPrepared = false;
};

class SQLiteTransaction 
{
public:
    SQLiteTransaction(sqlite3* db);
    ~SQLiteTransaction();

public:
    void Commit();

private:
    sqlite3*    _db = nullptr;
    bool        _isCommitted = false;
};