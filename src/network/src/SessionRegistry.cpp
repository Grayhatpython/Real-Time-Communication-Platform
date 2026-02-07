#include "network/NetworkPch.hpp"
#include "network/SessionRegistry.hpp"

namespace network
{
    SessionRegistry::SessionRegistry(int shardCount, SessionFactory sessionFactory)
        : _shards(shardCount), _sessionFactory(std::move(sessionFactory))
    {

    
    }
    
    void SessionRegistry::PostToShard(int32 shardId, ShardCommand command)
    {
        Shard& shard = _shards[shardId];
        {
            std::lock_guard<std::mutex> lock(shard.lock);
            shard.commandQueue.push(std::move(command));
        }

        if(shard.wakeup)
        {
            shard.wakeup();
        }
    }

    void SessionRegistry::ExecuteCommands(int32 shardId, EpollDispatcher& dispatcher)
    {
        Shard& shard = _shards[shardId];

        std::queue<ShardCommand> commandQueue;
        {
            std::lock_guard<std::mutex> lock(shard.lock);
            std::swap(commandQueue, shard.commandQueue);
        }

        while (commandQueue.empty() == false) 
        {
            ShardCommand& command = commandQueue.front();
            command(shard, dispatcher);
            commandQueue.pop();
        }
    }
}