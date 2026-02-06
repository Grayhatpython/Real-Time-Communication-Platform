#pragma once

#include "network/Session.hpp"
#include "engine/BinaryWriter.hpp"
#include "engine/BinaryReader.hpp"

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


class ClientSession : public network::Session
{
public:
    virtual void OnConnected() override
    {
        
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