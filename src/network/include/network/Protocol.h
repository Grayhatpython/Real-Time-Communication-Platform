#pragma once

#include "engine/CommonType.h"
#include "engine/BinaryReader.h"
#include "engine/BinaryWriter.h"

#include "network/Session.h"
#include "network/PacketHeader.h"
#include "network/SendBufferPool.h"

namespace Protocol
{
    enum class PacketId : uint16
    {
        // Auth
        C2S_Register = 100,
        C2S_Login    = 101,
        C2S_Resume   = 102,

        S2C_LoginOk  = 103,
        S2C_ResumeOk = 104,
        S2C_AuthFail = 105,

    };

    struct C2S_Login : public network::PacketHeader
    {
        std::string username;
        std::string password;
     
        bool Serialize(engine::BinaryWriter& bw) const
        {
            bw.Write<uint16>(static_cast<uint16>(username.size()));
            bw.Write(username.data(), static_cast<uint32>(username.size()));

            bw.Write<uint16>(static_cast<uint16>(password.size()));
            bw.Write(password.data(), static_cast<uint32>(password.size()));
            
            return true;
        }

        bool Deserialize(engine::BinaryReader& br)
        {
            uint16 size = 0;
            br.Read(size);
            username.resize(size);
            br.Read(username.data(), size);

            br.Read(size);
            password.resize(size);
            br.Read(password.data(), size);

            return true;
        }

        uint32 Size() const
        {
            uint32 totalSize = 0;

            totalSize += sizeof(network::PacketHeader);
            totalSize += sizeof(uint16); // username의 길이를 저장할 공간
            totalSize += static_cast<uint32>(username.size());
            
            totalSize += sizeof(uint16); // password의 길이를 저장할 공간
            totalSize += static_cast<uint32>(password.size());
            
            return totalSize;
        }
    };

    template<typename Packet>
    inline void SendPacket(std::shared_ptr<network::Session> session, PacketId packetId, const Packet& packet)
    {
        const uint32 totalSize = packet.Size();

        auto sendContext = std::make_shared<network::SendContext>();
        auto segment = network::SendBufferArena::Allocate(totalSize);

        engine::BinaryWriter bw(segment->ptr, totalSize);

        bw.Write<uint16>(totalSize);
        bw.Write(packetId);

        packet.Serialize(bw);

        sendContext->sendBuffer = segment->sendBuffer;
        sendContext->iovecBuf.iov_base = bw.GetBuffer();
        sendContext->iovecBuf.iov_len = bw.Size();
        sendContext->size = bw.Size();

        session->TryFlushSend(sendContext);
    }
}