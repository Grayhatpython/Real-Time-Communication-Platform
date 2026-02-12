#pragma once

#include "engine/CommonType.h"
#include "engine/BinaryReader.h"
#include "engine/BinaryWriter.h"

#include "network/Session.h"

namespace Protocol
{
    enum class PacketId : uint16
    {
        S2C_Spawn = 1001,
        S2C_Despawn = 1002,
    };

    template<typename Packet>
    inline void SendPacket(std::shared_ptr<network::Session> session, PacketId packetId, const Packet& packet)
    {
        const uint16 packetSize = static_cast<uint16>(sizeof(Packet));

        auto sendContext = std::make_shared<network::SendContext>();
        auto segment = network::SendBufferArena::Allocate(packetSize);

        engine::BinaryWriter bw(segment->ptr, packetSize);

        bw.Write<uint16>(packetSize);
        bw.Write(packetId);

        // packet.Serialize(bw);

        sendContext->sendBuffer = segment->sendBuffer;
        sendContext->iovecBuf.iov_base = bw.GetBuffer();
        sendContext->iovecBuf.iov_len = bw.Size();
        sendContext->size = bw.Size();

        session->TryFlushSend(sendContext);
    }
}