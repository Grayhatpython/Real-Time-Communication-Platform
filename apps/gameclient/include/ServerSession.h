#pragma once

#include "network/Session.h"

#include "gamelogic/Ship.h"
#include "gamelogic/Protocol.h"
#include "gamelogic/Math.h"

#include "engine/Logger.h"

class ServerSession : public network::Session
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

        uint16 pktSize = 0;
        Protocol::PacketId pktId;
        br.Read(pktSize);
        br.Read(pktId);

        switch (pktId)
        {
        case Protocol::PacketId::S2C_Spawn:
        {
            Protocol::SpawnPacket spawnPacket;
            if (spawnPacket.DeSerialize(br) == false) 
                return;

            // 이미 있으면 무시(중복 스폰 방지)
            if (_ships.find(spawnPacket.actorNetworkId) != _ships.end())
                return;

            // 로컬 Ship 생성
            std::shared_ptr<gamelogic::Ship> ship = std::make_shared<gamelogic::Ship>();
            ship->SetActorNetworkId(spawnPacket.actorNetworkId);
            ship->SetPosition(Vector2{spawnPacket.positionX, spawnPacket.positionY});
            ship->SetRotation(spawnPacket.rotation);
            ship->SetScale(spawnPacket.scale);

            _ships.emplace(spawnPacket.actorNetworkId, std::move(ship));

            EN_LOG_DEBUG("[Client] Spawn Ship id = {}",spawnPacket.actorNetworkId);

            break;
        }

        case Protocol::PacketId::S2C_Despawn:
        {
            Protocol::DespawnPacket despawnPacket;
            if (despawnPacket.DeSerialize(br) == false) 
                return;

            auto findIt = _ships.find(despawnPacket.actorNetworkId);
            if (findIt == _ships.end())
                return;

            _ships.erase(findIt);

            EN_LOG_DEBUG("[Client] Spawn Ship id = {}",despawnPacket.actorNetworkId);

            break;
        }

        default:
            break;
        }
    }

    virtual void OnSend() override
    {

    }

private:
    std::unordered_map<ActorNetworkId, std::shared_ptr<gamelogic::Ship>>_ships;
};