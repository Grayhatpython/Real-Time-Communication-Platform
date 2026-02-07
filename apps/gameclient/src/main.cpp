#include "Pch.hpp"
#include "ServerSession.hpp"
#include "SessionManager.hpp"

#include "network/NetworkCore.hpp"
#include "network/SendBufferPool.hpp"
int main()
{
    {
        engine::GlobalContext::GetInstance().Initialize();
        
        engine::ThreadManager* threadManager = engine::GlobalContext::GetInstance().GetThreadManager();

        //  SessionFactory 
        std::function<std::shared_ptr<ServerSession>(void)> sessionFactory = []() {
            return engine::MakeShared<ServerSession>();
        };
        std::unique_ptr<SessionManager> sessionManager = std::make_unique<SessionManager>();

        sessionManager->SetSessionFactory(sessionFactory);
        std::unique_ptr<network::Client> client = std::make_unique<network::Client>(sessionManager.get());
        client->Initialize();

        threadManager->RegisterExitCallback(
            engine::ThreadRole::Dispatch,
            [](){
                network::SendBufferArena::ThreadSendBufferClear();
            },
            "Network Clear Send Buffer"
        );

        //  network dispatch thread
        auto task = client->MakeDispatchTask();
        auto dispatchThraedHandle = threadManager->Spawn("NetworkDispatch",
            engine::ThreadRole::Dispatch,
            task);
        
        network::NetworkAddress targetAddress{"127.0.0.1", 8000};
        bool successed = client->Connect(targetAddress);

        if(successed == false)
            return RESULT_ERROR;

   
        auto start = std::chrono::high_resolution_clock::now();

        char input;

        std::cin >> input;

        if(input == 'c' || input == 'C')
        {
            client->StopDispatchTask();
            threadManager->RequestStop(dispatchThraedHandle);
            threadManager->Join(dispatchThraedHandle);

            threadManager->StopAllAndJoin();

            sessionManager.reset();
            network::SendBufferArena::SendBufferPoolClear();
            engine::GlobalContext::GetInstance().Clear();

        }

        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "시간 : " << duration.count() << " ms" << std::endl;
    }

    return 0;
} 







