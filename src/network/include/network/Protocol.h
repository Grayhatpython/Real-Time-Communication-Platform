#pragma once

#include "engine/CommonType.h"
#include "engine/BinaryReader.h"
#include "engine/BinaryWriter.h"
#include "engine/Logger.h"

#include "network/Session.h"
#include "network/SendBufferPool.h"

namespace Protocol
{
        //  TEMP
    #pragma pack(push, 1)
    struct PacketHeader
    {
        uint16 size;
        uint16 id;
    };
    #pragma pack(pop)

    enum class PacketId : uint16
    {
        // Auth
        C2S_Register = 100,
        C2S_Login    = 101,
        C2S_Resume   = 102,

        S2C_AuthOk   = 110,
        S2C_AuthFail = 111,

    };

    struct C2S_Register : public PacketHeader
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

    struct C2S_Login : public PacketHeader
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

    struct C2S_Resume
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

    struct S2C_AuthOk
    {
        size_t GetSerializedSize() const
        {
            return sizeof(uint64) + sizeof(int64) + (2 + token.size());
        }

        bool Serialize(engine::BinaryWriter& bw) const
        {
            if(bw.Write<uint64>(userId) == false || bw.Write<int64>(expiresAt) == false)
                return false;
            
            return bw.Write(token);
        }

        bool Deserialize(engine::BinaryReader& br)
        {
            if(br.Read(userId) == false || br.Read(expiresAt) == false)
                return false;

            return br.Read(token);
        }

        uint32 Size() const 
        {
            uint32 totalSize = 0;

            totalSize += sizeof(userId); 
            totalSize += sizeof(expiresAt); 
            totalSize += sizeof(uint32);     // token 길이를 저장할 공간
            totalSize += static_cast<uint32>(token.size());
            
            return totalSize;
        }

        uint64      userId = 0;
        int64       expiresAt = 0;
        std::string token; // Login/Register 시 발급
    };

    struct S2C_AuthFail
    {
        bool Serialize(engine::BinaryWriter& bw) const
        {
            return bw.Write<uint16>(reason);
        }

        bool Deserialize(engine::BinaryReader& br)
        {
            return br.Read(reason);
        }

        uint32 Size() const 
        {
            return sizeof(uint16);
        }

        uint16 reason = 0; // AuthFailReason 값을 uint16
    };

    template<typename Packet>
    inline void SendPacket(std::shared_ptr<network::Session> session, PacketId packetId, const Packet& packet)
    {
        const uint32 totalSize = sizeof(PacketHeader) + packet.Size();

        auto sendContext = std::make_shared<network::SendContext>();
        auto segment = network::SendBufferArena::Allocate(totalSize);

        engine::BinaryWriter bw(segment->ptr, totalSize);

        if(bw.Write<uint16>(totalSize) || bw.Write(packetId) == false)
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