#pragma once

#include "engine/BinaryReader.h"
#include "engine/BinaryWriter.h"
#include "engine/Logger.h"

#include "network/Session.h"
#include "network/PacketHeader.h"
#include "network/SendBufferPool.h"

enum class AuthOKType : uint16
{
    None = 0,
    Register,
    Login,
    Resume,
};

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
    int64 expiresAt = 0; // unix sec
    std::string token;   // base64url
};

namespace Protocol
{
    enum class PacketId : uint16
    {
        // Auth
        C2S_Register = 100,
        C2S_Login    = 101,
        C2S_Resume   = 102,

        S2C_AuthOk   = 110,
        S2C_AuthFail = 111,

    };

    struct C2S_Register : public network::PacketHeader
    {
        bool Serialize(engine::BinaryWriter& bw) const
        {
            if(bw.Write(username) == false || bw.Write(password) == false)
                return false;

            return true;
        }

        bool Deserialize(engine::BinaryReader& br)
        {
             if(br.Read(username) == false || br.Read(password) == false)
                return false;

            return true;
        }

        uint32 Size() const
        {
            uint32 totalSize = 0;

            totalSize += sizeof(uint32); // username의 길이를 저장할 공간
            totalSize += static_cast<uint32>(username.size());
            
            totalSize += sizeof(uint32); // password의 길이를 저장할 공간
            totalSize += static_cast<uint32>(password.size());
            
            return totalSize;
        }

        std::string username;
        std::string password;
    };

    struct C2S_Login : public network::PacketHeader
    {
        bool Serialize(engine::BinaryWriter& bw) const
        {
            if(bw.Write(username) == false || bw.Write(password) == false)
                return false;

            return true;
        }

        bool Deserialize(engine::BinaryReader& br)
        {
             if(br.Read(username) == false || br.Read(password) == false)
                return false;

            return true;
        }

        uint32 Size() const
        {
            uint32 totalSize = 0;

            totalSize += sizeof(uint32); // username의 길이를 저장할 공간
            totalSize += static_cast<uint32>(username.size());
            
            totalSize += sizeof(uint32); // password의 길이를 저장할 공간
            totalSize += static_cast<uint32>(password.size());
            
            return totalSize;
        }

        std::string username;
        std::string password;
    };

    struct C2S_Resume : public network::PacketHeader
    {
        bool Serialize(engine::BinaryWriter& bw) const
        {
            return bw.Write(token);
        }
        
        bool Deserialize(engine::BinaryReader& br)
        {
            return br.Read(token);
        }

        size_t Size() const
        {
            uint32 totalSize = 0;

            totalSize += sizeof(uint32); // username의 길이를 저장할 공간
            totalSize += static_cast<uint32>(token.size());

            return totalSize;
        }
        
        std::string token;
    };

    struct S2C_AuthOk : public network::PacketHeader
    {
        bool Serialize(engine::BinaryWriter& bw) const
        {
            if(bw.Write<uint64>(userId) == false || bw.Write<int64>(expiresAt) == false) 
                return false;
            
            if(bw.Write(token) == false || bw.Write(type) == false)
                return false;

            return true;
        }

        bool Deserialize(engine::BinaryReader& br)
        {
            if(br.Read(userId) == false || br.Read(expiresAt) == false)
                return false;

            if(br.Read(token) == false || br.Read(type) == false)
                return false;

            return true;
        }

        uint32 Size() const 
        {
            uint32 totalSize = 0;

            totalSize += sizeof(userId); 
            totalSize += sizeof(expiresAt); 
            totalSize += sizeof(uint32);     // token 길이를 저장할 공간
            totalSize += static_cast<uint32>(token.size());
            totalSize += sizeof(AuthOKType);
            
            return totalSize;
        }

        uint64      userId = 0;
        int64       expiresAt = 0;
        std::string token; // Login/Register 시 발급
        AuthOKType  type;
    };

    struct S2C_AuthFail : public network::PacketHeader
    {
        bool Serialize(engine::BinaryWriter& bw) const
        {
            return bw.Write(reason);
        }

        bool Deserialize(engine::BinaryReader& br)
        {
            return br.Read(reason);
        }

        uint32 Size() const 
        {
            return sizeof(AuthFailReason);
        }

        AuthFailReason reason = AuthFailReason::None;
    };

    template<typename Packet>
    inline void SendPacket(std::shared_ptr<network::Session> session, PacketId packetId, const Packet& packet)
    {
        const uint32 totalSize = sizeof(network::PacketHeader) + packet.Size();

        auto sendContext = std::make_shared<network::SendContext>();
        auto segment = network::SendBufferArena::Allocate(totalSize);

        engine::BinaryWriter bw(segment->ptr, totalSize);

        if(bw.Write<uint16>(totalSize) == false || bw.Write(packetId) == false)
        {
            EN_LOG_DEBUG("PacketId[{}] PacketHeader Serialize Failed", static_cast<uint16>(packetId));
            return;
        }

        //  TODO
        if(packet.Serialize(bw) == false)
        {
            EN_LOG_DEBUG("PacketId[{}] Payload Serialize Failed", static_cast<uint16>(packetId));
            return;
        }

        sendContext->sendBuffer = segment->sendBuffer;
        sendContext->iovecBuf.iov_base = bw.GetBuffer();
        sendContext->iovecBuf.iov_len = bw.Size();
        sendContext->size = bw.Size();

        session->TryFlushSend(sendContext);
    }
}