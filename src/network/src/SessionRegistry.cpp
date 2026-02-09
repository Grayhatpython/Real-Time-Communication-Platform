#include "network/NetworkPch.h"
#include "network/SessionRegistry.h"
#include "network/Session.h"
#include "network/NetworkDispatcher.h"
#include "network/NetworkUtils.h"
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

    void SessionRegistry::ExecuteCommands(int32 shardId, std::shared_ptr<EpollDispatcher>& dispatcher)
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

    void SessionRegistry::ProcessAccepted(int32 shardId, SocketFd clientSocketFd)
    {
        PostToShard(shardId, [this, shardId, clientSocketFd](Shard &shard,  std::shared_ptr<EpollDispatcher> &dispatcher) {
            auto newSession = CreateSession();
            if (newSession == nullptr) 
            { 
                ::close(clientSocketFd); 
                return; 
            }

            uint64 sessionId = shard.AllocateSessionId(uint16_t(shardId));
            newSession->SetSocketFd(clientSocketFd);
            newSession->SetSessionId(sessionId);
            newSession->SetShardID(shardId);
            newSession->SetOwner(this);
            newSession->SetState(SessionState::Connected);

            // fd를 오너 dispatcher의 epoll에 등록
            if (dispatcher->Register(newSession) == false) 
            {
                ::close(clientSocketFd);
                return;
            }

            if(NetworkUtils::SetNonBlocking(clientSocketFd) == false || NetworkUtils::SetTcpNoDelay(clientSocketFd, true) == false) 
            {
                dispatcher->UnRegister(newSession);
                ::close(clientSocketFd);
                return;
            }

            newSession->ProcessConnect();
            shard.sessions.emplace(sessionId, std::move(newSession)); 
        });
    }

    void SessionRegistry::ConnectAsync(int32 shardId, NetworkAddress &targetAddress)
    {
        PostToShard(shardId, [this, shardId, targetAddress](Shard &shard, std::shared_ptr<EpollDispatcher> &dispatcher) {

                auto newSession = CreateSession();
                if (newSession == nullptr) 
                { 
                    return; 
                }
                
                SocketFd socketFd = NetworkUtils::CreateSocketFd();
                if(socketFd == INVALID_SOCKET_FD_VALUE)
                    return;

                if(NetworkUtils::SetNonBlocking(socketFd) == false)
                {
                    ::close(socketFd);
                    socketFd = INVALID_SOCKET_FD_VALUE;
                    return;
                }

                uint64 sessionId = shard.AllocateSessionId(static_cast<uint16>(shardId));
                newSession->SetSocketFd(socketFd);
                newSession->SetSessionId(sessionId);
                newSession->SetShardID(shardId);
                newSession->SetOwner(this);
                newSession->SetState(SessionState::ConnectPending);

		        struct sockaddr_in serverAddress = targetAddress.GetSocketAddress();
                int32 ret = ::connect(socketFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

                if (ret == RESULT_OK) 
                {
                    // 즉시 연결됨
                    newSession->SetState(SessionState::Connected);

                    // fd를 오너 dispatcher의 epoll에 등록
                    if (dispatcher->Register(newSession) == false) 
                    {
                        ::close(socketFd);
                        socketFd = INVALID_SOCKET_FD_VALUE;
                        return;
                    }

                    shard.sessions.emplace(sessionId, std::move(newSession)); 
                    newSession->ProcessConnect();
                    return;
                }

                if (ret == RESULT_ERROR && errno == EINPROGRESS) 
                {
                    // fd를 오너 dispatcher의 epoll에 등록
                    if (dispatcher->Register(newSession) == false) 
                    {
                        ::close(socketFd);
                        socketFd = INVALID_SOCKET_FD_VALUE;
                        return;
                    }
                    
                    // 연결 진행중 → EPOLLOUT로 완료 감지
                    dispatcher->EnableConnectEvent(newSession);
                    shard.sessions.emplace(sessionId, std::move(newSession)); 
                    return;
                }

                // 즉시 실패
                ::close(socketFd);
                socketFd = INVALID_SOCKET_FD_VALUE;
        });
    }
}

