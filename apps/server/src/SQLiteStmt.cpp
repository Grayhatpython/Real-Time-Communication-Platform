#include "Pch.h"
#include "SQLiteStmt.h"

#include <sqlite3.h>

SQLiteStmt::SQLiteStmt(sqlite3* db, const std::string& sql)
    : _db(db)
{
    int32 ret = sqlite3_prepare_v2(_db, sql.c_str(), -1, &_stmt, nullptr);

    if(ret == SQLITE_OK && _stmt)
        _isPrepared = true;
    else
        _isPrepared = false;
}

SQLiteStmt::~SQLiteStmt()
{
    if (_stmt)
    {
        sqlite3_finalize(_stmt);
        _stmt = nullptr;
    }
}

bool SQLiteStmt::BindText(int32 index, const std::string& text)
{
    return sqlite3_bind_text(_stmt, index, text.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK;
}

bool SQLiteStmt::BindInt64(int32 index, int64 value)
{
    return sqlite3_bind_int64(_stmt, index, value) == SQLITE_OK;
}

bool SQLiteStmt::BindInt(int32 index, int32 value)
{
    return sqlite3_bind_int(_stmt, index, value) == SQLITE_OK;
}

bool SQLiteStmt::BindDouble(int32 index, double value)
{
    return sqlite3_bind_double(_stmt, index, value) == SQLITE_OK; 
}

bool SQLiteStmt::BindBlob(int32 index, const void* data, int32 size) 
{
    //  std::vector<char> imageData = LoadImageFile("*.jpg");
    //  stmt.BindBlob(1, imageData.data(), static_cast<int32>(imageData.size()));

    return sqlite3_bind_blob(_stmt, index, data, size, SQLITE_TRANSIENT) == SQLITE_OK;
}

int32 SQLiteStmt::GetInt(int32 columnIndex)
{
    return sqlite3_column_int(_stmt, columnIndex);
}

int64 SQLiteStmt::GetInt64(int32 columnIndex)
{
    return sqlite3_column_int64(_stmt, columnIndex);
}

double SQLiteStmt::GetDouble(int32 columnIndex)
{
    return sqlite3_column_double(_stmt, columnIndex);
}

std::string SQLiteStmt::GetText(int32 columnIndex)
{
    const unsigned char* text = sqlite3_column_text(_stmt, columnIndex);
    return text ? reinterpret_cast<const char*>(text) : "";
}

 const void* SQLiteStmt::GetBlob(int32 columnIndex, int32& size)
 {
    size = sqlite3_column_bytes(_stmt, columnIndex); // 데이터 크기 확인
    return sqlite3_column_blob(_stmt, columnIndex);    // 데이터 포인터 반환

    //  int32 size = 0;
    //  const void* ptr = stmt.GetBlob(0, size);
    //  std::vector<char> retrievedData((char*)ptr, (char*)ptr + size);
}

bool SQLiteStmt::StepRow()
{
    if (IsPrepared() == false) 
        return false;
    return sqlite3_step(_stmt) == SQLITE_ROW ? true : false;
}

bool SQLiteStmt::StepDone()
{
    if (IsPrepared() == false) 
        return false;
    return sqlite3_step(_stmt) == SQLITE_DONE;
}

