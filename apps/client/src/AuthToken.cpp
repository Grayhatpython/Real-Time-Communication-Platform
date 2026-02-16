#include "Pch.h"
#include "AuthToken.h"

#include <chrono>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <nlohmann/json.hpp>

AuthTokenStore::AuthTokenStore(std::string path)
{

}

bool AuthTokenStore::Load(AuthToken &authToken)
{
    std::ifstream ifs(_path, std::ios::binary);
    if(!ifs)
        return false;

    nlohmann::json json;
    ifs >> json;

    authToken.userId = json.value("userId", (uint64)0);
    authToken.last_seen_at = json.value("last_seen_at", (int64)0);
    authToken.expires_at = json.value("expires_at", (int64)0);
    authToken.token = json.value("token", std::string{});

    return true;
}

bool AuthTokenStore::Save(const AuthToken &authToken)
{
    std::filesystem::create_directories(std::filesystem::path(_path).parent_path());

    nlohmann::json json;
    json["userId"] = authToken.userId;
    json["last_seen_at"] = authToken.last_seen_at;
    json["expires_at"] = authToken.expires_at;
    json["token"] = authToken.token;

    std::ofstream ofs(_path, std::ios::binary | std::ios::trunc);
    if (ofs.is_open() == false)
        return false;

    // pretty print(2 spaces) + trailing newline
    ofs << json.dump(2) << "\n";
    ofs.flush();

    // flush/close 이후 상태 체크
    return ofs.good();
}

bool AuthTokenStore::Clear()
{
    return true;
}

int64 AuthTokenStore::NowSeconds()
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
