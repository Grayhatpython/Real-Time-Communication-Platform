#pragma once

#include "gamelogic/Types.h"

#include "engine/BinaryWriter.h"
#include "engine/BinaryReader.h"

#include "network/Session.h"
#include "network/PacketHeader.h"
#include "network/SendBufferPool.h"

namespace Protocol
{
    enum class PacketId : uint16
    {
        S2C_Spawn = 1001,
        S2C_Despawn = 1002,
    };


    #pragma pack(push, 1)
    struct SpawnPacket : network::PacketHeader
    {
        bool Serialize(engine::BinaryWriter& bw) const
        {
            return bw.Write<ActorNetworkId>(actorNetworkId)
                && bw.Write<float>(positionX)
                && bw.Write<float>(positionY)
                && bw.Write<float>(rotation)
                && bw.Write<float>(scale);
        }

        bool DeSerialize(engine::BinaryReader& br)
        {
            return br.Read(actorNetworkId)
                && br.Read(positionX)
                && br.Read(positionY)
                && br.Read(rotation)
                && br.Read(scale);
        }
        
        ActorNetworkId actorNetworkId = 0;
        float positionX = 0.0f;
        float positionY = 0.0f;
        float rotation = 0.0f;
        float scale = 0.0f;
    };

    struct DespawnPacket : network::PacketHeader
    {
        
        bool Serialize(engine::BinaryWriter& bw) const
        {
            return bw.Write<ActorNetworkId>(actorNetworkId);
        }
        
        bool DeSerialize(engine::BinaryReader& br)
        {
            return br.Read(actorNetworkId);
        }

        ActorNetworkId actorNetworkId = 0;
    };
    #pragma pack(pop)

    template<typename Packet>
    inline void SendPacket(std::shared_ptr<network::Session> session, PacketId packetId, const Packet& packet)
    {
        const uint16 packetSize = static_cast<uint16>(sizeof(Packet));

        auto sendContext = std::make_shared<network::SendContext>();
        auto segment = network::SendBufferArena::Allocate(packetSize);

        engine::BinaryWriter bw(segment->ptr, packetSize);

        bw.Write<uint16>(packetSize);
        bw.Write(packetId);

        packet.Serialize(bw);

        sendContext->sendBuffer = segment->sendBuffer;
        sendContext->iovecBuf.iov_base = bw.GetBuffer();
        sendContext->iovecBuf.iov_len = bw.Size();
        sendContext->size = bw.Size();

        session->TryFlushSend(sendContext);
    }
}