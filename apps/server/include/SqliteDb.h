#pragma once

class sqlite3;
class SqliteDb 
{
public:
    SqliteDb() =default;
    ~SqliteDb();

    SqliteDb(const SqliteDb&) = delete;
    SqliteDb& operator=(const SqliteDb&) = delete;

public:
    bool Open(const std::string& filePath);
    void Close();
    bool Exec(const std::string& sql);

public:
    sqlite3* GetDBHandle() const { return _db; }

private:
    sqlite3* _db = nullptr;
};