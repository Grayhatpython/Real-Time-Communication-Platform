#include "Pch.hpp"
#include "NetworkCore.hpp"
#include "Session.hpp"
#include "Acceptor.hpp"
#include "NetworkUtils.hpp"
#include "SessionManager.hpp"
#include "NetworkDispatcher.hpp"
#include "Logger.hpp"

namespace servercore
{
    INetworkCore::INetworkCore(std::function<std::shared_ptr<Session>()> sessionFactory)
    {
        Initialize(sessionFactory);
    }

    INetworkCore::~INetworkCore()
    {

    }

    void INetworkCore::Stop()
    {
        //  스레드 종료 시그널 -> 스레드 작업 마무리
        GThreadManager->Stop();

        _isRunning.store(false, std::memory_order_release);
    
        // shutdown 시그널 -> epoll 이벤트 작업 종료
        auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
        epollDispatcher->PostCoreShutdown();

        //  Network Dispatch Thread 종료            
        if(_dispatchThread.joinable() == true)
            _dispatchThread.join();

        // 연결된 Session Socket Close 및 epoll 등록 해제
        GSessionManager->Clear();

        // core Event에 사용된 eventfd 및 epollfd 삭제
        epollDispatcher->Stop();

        //  async_logger 처리 및 스레드 작업 마무리
        Logger::Shutdown();

        // 할당된 메모리 정리 ( 메모리풀 )
        GlobalContext::GetInstance().Clear();
    }

    void INetworkCore::NetworkDispatch()
    { 
        _dispatchThread = std::move(std::thread([=](){

            ThreadManager::InitializeThreadLocal("NetworkDispatch");

            NC_LOG_INFO("Thread Started");

            while (_isRunning.load(std::memory_order_acquire) == true)
            {
                auto dispatchResult = _networkDispatcher->Dispatch();

                //  TODO
                if(dispatchResult == servercore::DispatchResult::InvalidDispatcher || 
                    dispatchResult == servercore::DispatchResult::ExitRequested)
                {
                    
                }         
            }

            ThreadManager::DestroyThreadLocal();

            NC_LOG_INFO("Thread finished");

        }));
    }

    void INetworkCore::Initialize(std::function<std::shared_ptr<Session>()> sessionFactory)
    {
        NetworkUtils::Initialize();
        GlobalContext::GetInstance().Initialize();
        GSessionManager->SetSessionFactory(sessionFactory);

        {
            _networkDispatcher = std::make_shared<EpollDispatcher>();
            auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
            
            if(epollDispatcher)
            {
                epollDispatcher->Initialize();
            }
        }

        NC_LOG_INFO("Network Core Initialized");
    }

    Server::Server(std::function<std::shared_ptr<Session>()> sessionFactory)
        : INetworkCore(sessionFactory), _acceptor(std::make_shared<Acceptor>())
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

        _acceptor->_networkDispatcher = _networkDispatcher;
        if(_acceptor->Start(port) == false)
        {
            return false;
        }
        
        _port = port;
        _isRunning.store(true, std::memory_order_release);
    
        NC_LOG_INFO("Server port:{} Started", _port);

        NetworkDispatch();
        return true;
    }

    void Server::Stop()
    {
        if(_isRunning.load(std::memory_order_acquire) == false)
            return;
        
        NC_LOG_INFO("Server Stopping");

        if(_acceptor)
            _acceptor->Stop();

        INetworkCore::Stop();

        NC_LOG_INFO("Server Stopped");
    }
    
    Client::Client(std::function<std::shared_ptr<Session>()> sessionFactory)
        : INetworkCore(sessionFactory)
    {

    }

    Client::~Client()
    {

    }

    bool Client::Connect(NetworkAddress& targetAddress, int32 connectionCount)
    {
        NC_LOG_INFO("Client target Address:{} port:{} Connecting", targetAddress.GetIpStringAddress(), targetAddress.GetPort());

        std::vector<uint64> connectedSessions;  

        for (int i = 0; i < connectionCount; ++i)
        {
            auto session = GSessionManager->CreateSession();
     
            GSessionManager->AddSession(session);
            connectedSessions.push_back(session->GetSessionId());

            session->_networkDispatcher = _networkDispatcher;

            if (session->Connect(targetAddress) == false)
            {
                // 1) 이번 세션은 즉시 파기(정상 진입 전)
                GSessionManager->AbortSession(session->GetSessionId());

                // 2) 이미 만들어진 다른 세션들 정리
                for (auto id : connectedSessions)
                {
                    if (id == session->GetSessionId()) 
                        continue;

                    auto epollDispatcher = std::static_pointer_cast<EpollDispatcher>(_networkDispatcher);
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

        NC_LOG_INFO("Client Stopping");

        INetworkCore::Stop();

        NC_LOG_INFO("Server Stopped");
    }
    
}