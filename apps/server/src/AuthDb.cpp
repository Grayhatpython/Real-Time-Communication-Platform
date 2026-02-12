#include "Pch.h"
#include "AuthDb.h"
#include "SqliteDb.h"

#include <sstream>
#include <fstream>
#include <filesystem>

#include "engine/Logger.h"

AuthDb::AuthDb()
    :   _sqliteDb(new SqliteDb())
{

}

AuthDb::~AuthDb()
{
    if(_sqliteDb)
    {
        delete _sqliteDb;
        _sqliteDb = nullptr;
    }
}

bool AuthDb::Initialize(const std::string& dbPath, const std::string& schemaPath)
{
    // DB 폴더 생성
    std::filesystem::path path(dbPath);
    if (path.has_parent_path() == true)
    {
        std::filesystem::create_directories(path.parent_path());
    }

    if(_sqliteDb->Open(dbPath) == false)
        return false;

    std::ifstream ifs(schemaPath, std::ios::binary);
    if (!ifs)
    {
        EN_LOG_ERROR("Failed to open file: {}", schemaPath);
        return false;
    }

    std::ostringstream oss;
    oss << ifs.rdbuf();

    std::string schemaSql = oss.str();
    
    return _sqliteDb->Exec(schemaSql);
}