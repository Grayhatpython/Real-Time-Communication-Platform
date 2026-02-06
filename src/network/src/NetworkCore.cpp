#include "network/NetworkPch.hpp"
#include "network/NetworkCore.hpp"
#include "network/Session.hpp"
#include "network/Acceptor.hpp"
#include "network/NetworkUtils.hpp"
#include "network/NetworkDispatcher.hpp"
#include "network/SessionRegistry.hpp"
#include "network/SendBufferPool.hpp"

namespace network
{
    NetworkCore::NetworkCore(ISessionRegistry*  sessionRegistry)
        : _sessionRegistry(sessionRegistry)
    {
        Initialize();
    }

    NetworkCore::~NetworkCore()
    {

    }

    void NetworkCore::Stop()
    {
        //  스레드 종료 시그널 -> 스레드 작업 마무리
        engine::GlobalContext::GetInstance().GetThreadPool()->Stop();

        _isRunning.store(false, std::memory_order_release);
    
        // shutdown 시그널 -> epoll 이벤트 작업 종료
        auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
        epollDispatcher->PostCoreShutdown();

        //  Network Dispatch Thread 종료            
        if(_dispatchThread.joinable() == true)
            _dispatchThread.join();

        // 연결된 Session Socket Close 및 epoll 등록 해제
        _sessionRegistry->Clear();

        // core Event에 사용된 eventfd 및 epollfd 삭제
        epollDispatcher->Stop();
     
        // 할당된 메모리 정리 ( 메모리풀 )
        engine::GlobalContext::GetInstance().Clear();
   
    }

    void NetworkCore::NetworkDispatch()
    { 
        _dispatchThread = std::move(std::thread([=](){

            engine::ThreadManager::InitializeThreadLocal("NetworkDispatch");

            EN_LOG_INFO("Thread Started");

            while (_isRunning.load(std::memory_order_acquire) == true)
            {
                auto dispatchResult = _networkDispatcher->Dispatch();

                //  TODO
                if(dispatchResult == DispatchResult::InvalidDispatcher || 
                    dispatchResult == DispatchResult::ExitRequested)
                {
                    
                }         
            }

            auto destroyTLSCallback = [](){
                SendBufferArena::ThreadSendBufferClear();
            };

            engine::ThreadManager::RegisterDestroyThreadLocal(destroyTLSCallback);
            engine::ThreadManager::DestroyThreadLocal();

            EN_LOG_INFO("Thread finished");

        }));
    }

    void NetworkCore::Initialize()
    {
        NetworkUtils::Initialize();
        engine::GlobalContext::GetInstance().Initialize();

        auto destroyTLSCallback = [](){
            SendBufferArena::ThreadSendBufferClear();
            SendBufferArena::SendBufferPoolClear();
        };

        engine::GlobalContext::GetInstance().GetThreadPool()->RegisterDestroyThreadLocal(destroyTLSCallback);

        {
            _networkDispatcher = std::make_shared<EpollDispatcher>();
            auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
            
            if(epollDispatcher)
            {
                epollDispatcher->Initialize();
            }
        }

        EN_LOG_INFO("Network Core Initialized");
    }

    Server::Server(ISessionRegistry*  sessionRegistry)
        : NetworkCore(sessionRegistry), _acceptor(std::make_shared<Acceptor>())
    {

    }

    Server::~Server()
    {
        
    }

    bool Server::Start(uint16 port)
    {
        if(_acceptor == nullptr)
            return false;

        if(_networkDispatcher == nullptr)
            return false;

        if(_sessionRegistry == nullptr)
            return false;

        _acceptor->SetNetworkDispatcher(_networkDispatcher);
        _acceptor->SetSessionRegistry(_sessionRegistry);
        
        auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
        epollDispatcher->SetSessionRegistry(_sessionRegistry);

        if(_acceptor->Start(port) == false)
        {
            return false;
        }
        
        _port = port;
        _isRunning.store(true, std::memory_order_release);
    
        EN_LOG_INFO("Server port:{} Started", _port);

        NetworkDispatch();
        return true;
    }

    void Server::Stop()
    {
        if(_isRunning.load(std::memory_order_acquire) == false)
            return;
        
        EN_LOG_INFO("Server Stopping");

        if(_acceptor)
            _acceptor->Stop();

        NetworkCore::Stop();

        EN_LOG_INFO("Server Stopped");
    }
    
    Client::Client(ISessionRegistry*  sessionRegistry)
        : NetworkCore(sessionRegistry)
    {

    }

    Client::~Client()
    {

    }

    bool Client::Connect(NetworkAddress& targetAddress, int32 connectionCount)
    {
        if(_networkDispatcher == nullptr)
            return false;

        if(_sessionRegistry == nullptr)
            return false;
            
        EN_LOG_INFO("Client target Address:{} port:{} Connecting", targetAddress.GetIpStringAddress(), targetAddress.GetPort());
            
        auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
        epollDispatcher->SetSessionRegistry(_sessionRegistry);

        std::vector<uint64> connectedSessions;  

        for (int i = 0; i < connectionCount; ++i)
        {
            auto session = _sessionRegistry->CreateSession();
     
            _sessionRegistry->AddSession(session);
            connectedSessions.push_back(session->GetSessionId());

            session->SetNetworkDispatcher(_networkDispatcher);

            if (session->Connect(targetAddress) == false)
            {
                // 1) 이번 세션은 즉시 파기(정상 진입 전)
                _sessionRegistry->AbortSession(session->GetSessionId());

                // 2) 이미 만들어진 다른 세션들 정리
                for (auto id : connectedSessions)
                {
                    if (id == session->GetSessionId()) 
                        continue;

                    epollDispatcher->PostRemoveSessionEvent(id);
                }

                return false;
            }
        }

        _isRunning.store(true, std::memory_order_release);

        NetworkDispatch();

        return true;
    }

    void Client::Stop()
    {
        if(_isRunning.load(std::memory_order_acquire) == false)
            return;

        EN_LOG_INFO("Client Stopping");

        NetworkCore::Stop();

        EN_LOG_INFO("Client Stopped");
    }
    
}