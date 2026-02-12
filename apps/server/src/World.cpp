// #include "Pch.h"
// #include "World.h"
// #include "ClientSession.h"

// #include "gamelogic/Math.h"
// #include "gamelogic/Protocol.h"

// #include "engine/Logger.h"

// void World::Enter(std::shared_ptr<ClientSession> newClient)
// {
//     const ActorNetworkId newActorNetworkId = _generateActorNetworkId++;

//     Player player;
//     player.ownerSession = newClient;
//     player.ship = std::make_shared<gamelogic::Ship>();
//     player.ship->SetActorNetworkId(newActorNetworkId);
//     player.ship->SetPosition(Vector2(512.f, 384.f));

//     _players.emplace(newActorNetworkId, std::move(player));
//     _sessionToActorNetworkIdMap[newClient] = newActorNetworkId;

//     {
//         //  새로 접속한 클라이언트에게 기존의 접속되어 있던 플레이어들 스폰 전송
//         for(auto& [actorNetworkId, player] : _players)
//         {
//             if(actorNetworkId == newActorNetworkId)
//                 continue;

//             const auto& ship = player.ship;
//             Protocol::SpawnPacket spawnPacket;
//             spawnPacket.actorNetworkId = ship->GetActorNetworkId();
//             spawnPacket.positionX = ship->GetPosition().x;
//             spawnPacket.positionY = ship->GetPosition().y;
//             spawnPacket.rotation = ship->GetRotation();
//             spawnPacket.scale = ship->GetScale();

//             Protocol::SendPacket(newClient, Protocol::PacketId::S2C_Spawn, spawnPacket);
//         }
//     }

//     {
//         //  모든 기존 클라이언트들에게: "새 플레이어 스폰" 브로드캐스트
//         const auto& newShip = _players[newActorNetworkId].ship;
//         BroadcastSpawn(newShip);
//     }

//     EN_LOG_DEBUG("[Client Actor {} Enter]",newActorNetworkId);
// }

// void World::Leave(std::shared_ptr<ClientSession> client)
// {
//     auto findIt = _sessionToActorNetworkIdMap.find(client);
//     if (findIt == _sessionToActorNetworkIdMap.end())
//         return;

//     const ActorNetworkId actorNetworkId = findIt->second;

//     // 서버 월드에서 제거
//     _sessionToActorNetworkIdMap.erase(findIt);
//     _players.erase(actorNetworkId);

//     // 남은 클라들에게 디스폰 브로드캐스트
//     BroadcastDespawn(actorNetworkId);

//     EN_LOG_DEBUG("[Client Actor {} Leave]",actorNetworkId);

// }

// void World::BroadcastSpawn(const std::shared_ptr<gamelogic::Ship>& ship)
// {
//     Protocol::SpawnPacket spawnPacket;
//     spawnPacket.actorNetworkId = ship->GetActorNetworkId();
//     spawnPacket.positionX = ship->GetPosition().x;
//     spawnPacket.positionY = ship->GetPosition().y;
//     spawnPacket.rotation = ship->GetRotation();
//     spawnPacket.scale = ship->GetScale();

//     for(auto& [clientSession, actorNetworkId] : _sessionToActorNetworkIdMap)
//     {
//         Protocol::SendPacket(clientSession, Protocol::PacketId::S2C_Spawn, spawnPacket);
//     }
// }

// void World::BroadcastDespawn(ActorNetworkId actorNetworkId)
// {
//     Protocol::DespawnPacket despawnPacket;
//     despawnPacket.actorNetworkId = actorNetworkId;

//     for(auto& [clientSession, actorNetworkId] : _sessionToActorNetworkIdMap)
//     {
//         Protocol::SendPacket(clientSession, Protocol::PacketId::S2C_Despawn, despawnPacket);
//     }
// }

// ActorNetworkId  World::GetOwnedActorId(std::shared_ptr<ClientSession> client) const
// {
//     auto findIt = _sessionToActorNetworkIdMap.find(client);
//     if (findIt == _sessionToActorNetworkIdMap.end()) 
//         return 0;

//     return findIt->second;
// }