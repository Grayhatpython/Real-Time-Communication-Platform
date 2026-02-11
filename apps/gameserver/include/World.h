#pragma once
#include "gamelogic/Ship.h"

class ClientSession;
class World
{
public:
   
public:
    void Enter(std::shared_ptr<ClientSession> newClient);
    void Leave(std::shared_ptr<ClientSession> client);

    void BroadcastSpawn(const std::shared_ptr<gamelogic::Ship>& ship);
    void BroadcastDespawn(ActorNetworkId actorNetworkId);

public:
    ActorNetworkId  GetOwnedActorId(std::shared_ptr<ClientSession> client) const;

private:
    //  TEMP    
    ActorNetworkId _generateActorNetworkId = 1;

    struct Player
    {
        std::shared_ptr<ClientSession> ownerSession;
        std::shared_ptr<gamelogic::Ship> ship;
    };

    std::unordered_map<ActorNetworkId, Player> _players;
    std::unordered_map<std::shared_ptr<ClientSession>, ActorNetworkId> _sessionToActorNetworkIdMap;
};