#pragma once
#include <cstdint>
#include <functional>
#include <memory>

namespace network
{
    class Session;
    class EpollDispatcher;
    class SessionRegistry 
    {
    public:
        struct Shard;
        using SessionFactory = std::function<std::shared_ptr<Session>()>;
        using ShardCommand = std::function<void(struct Shard&, EpollDispatcher&)>;

        struct Shard
        {
            std::unordered_map<uint64, std::shared_ptr<Session>> sessions;
            
            std::mutex lock;
            std::queue<ShardCommand> commandQueue;

            std::function<void(void)> wakeup;

            uint64 sessionIdSequenceNumber = 0;

            uint64 AllocateSessionId(uint16 shardId)
            {
                ++sessionIdSequenceNumber;

                uint64 sessionId = (sessionIdSequenceNumber & 0x0000FFFFFFFFFFFFULL);
                return (static_cast<uint64>(shardId) << 48) | sessionId;
            }
        };

    public:
        SessionRegistry(int shardCount, SessionFactory sessionFactory);

        void    PostToShard(int32 shardId, ShardCommand command);
        void    ExecuteCommands(int32 shardId, EpollDispatcher& dispatcher);

    public:
        std::shared_ptr<Session>    FindSession(int32 shardId, uint64 sessionId)
        {
            auto& sessions = _shards[shardId].sessions;
            auto findIt = sessions.find(sessionId);
            return (findIt == sessions.end()) ? nullptr : findIt->second;
        }

        void                        RemoveSession(int32 shardId, uint64 sessionId) { _shards[shardId].sessions.erase(sessionId); }
        std::shared_ptr<Session>    CreateSession() { return _sessionFactory ? _sessionFactory() : nullptr; }   

    public:
        void    SetShardWakeup(int32 shardId, std::function<void(void)> wakeup)
        {
            _shards[shardId].wakeup = std::move(wakeup);
        }
        int32   GetShardCount() const { return static_cast<int32>(_shards.size()); }
    
    private:
        SessionFactory      _sessionFactory;
        std::vector<Shard>  _shards;
    };
}