#pragma once

#include "network/Session.h"
#include "network/SendBufferPool.h"

#include "engine/BinaryWriter.h"
#include "engine/BinaryReader.h"

enum class PacketId : uint16_t
{
    Stat = 200,
};

#pragma pack(push, 1)
struct StatPacket : PacketHeader
{
    uint64 playerId;
    uint32 playerHp;
    uint32 playerMp;

    bool Serialize(engine::BinaryWriter& bw) const
    {
        return bw.Write<uint64>(playerId)
            && bw.Write<uint32>(playerHp)
            && bw.Write<uint32>(playerMp);
    }

    bool DeSerialize(engine::BinaryReader& br)
    {
        return br.Read(playerId)
            && br.Read(playerHp)
            && br.Read(playerMp);
    }
};
#pragma pack(pop)


class ServerSession : public network::Session
{
public:
    virtual void OnConnected() override
    {
        auto sendContext = std::make_shared<network::SendContext>();

        uint16 size = sizeof(StatPacket);
        auto segment = network::SendBufferArena::Allocate(size);

        engine::BinaryWriter bw(segment->ptr, size);
        bw.Write(size);
        bw.Write(PacketId::Stat);
        StatPacket statPacket;
        statPacket.playerId = 10;
        statPacket.playerHp = 200;
        statPacket.playerMp = 100;
        statPacket.Serialize(bw);

        sendContext->sendBuffer = segment->sendBuffer;
        sendContext->iovecBuf.iov_base = bw.GetBuffer();
        sendContext->iovecBuf.iov_len = bw.Size();
        sendContext->size = bw.Size();

        TryFlushSend(sendContext);
    }

    virtual void OnDisconnected() override
    {

    }

    virtual void OnRecv(BYTE* buffer, int32 numOfBytes) override
    {
        engine::BinaryReader br(buffer, numOfBytes);
        br.MoveReadPos(sizeof(PacketHeader));

        StatPacket statPacket;
        statPacket.DeSerialize(br);

        std::cout << statPacket.playerId << " " << statPacket.playerMp << " " << std::endl;
    }

    virtual void OnSend() override
    {

    }

private:

};