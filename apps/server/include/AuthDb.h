#pragma once


class SqliteDb;
class AuthDb 
{
public:
    AuthDb();
    ~AuthDb();

public:
    bool Initialize(const std::string& dbPath, const std::string& schemaPath);

public:
    SqliteDb* GetDBHandle() const { return _sqliteDb; }
    
private:
    SqliteDb* _sqliteDb = nullptr; 
};