#include "Pch.hpp"
#include "ClientSession.hpp"
#include "SessionManager.hpp"

#include "network/NetworkCore.hpp"
#include "network/SendBufferPool.hpp"
int main()
{
    {
        engine::GlobalContext::GetInstance().Initialize();
        
        engine::ThreadManager* threadManager = engine::GlobalContext::GetInstance().GetThreadManager();

        //  SessionFactory 
        std::function<std::shared_ptr<ClientSession>(void)> sessionFactory = []() {
            return engine::MakeShared<ClientSession>();
        };
        std::unique_ptr<SessionManager> sessionManager = std::make_unique<SessionManager>();

        sessionManager->SetSessionFactory(sessionFactory);
        std::unique_ptr<network::Server> server = std::make_unique<network::Server>(sessionManager.get());
        server->Initialize();

        threadManager->RegisterExitCallback(
            engine::ThreadRole::Dispatch,
            [](){
                network::SendBufferArena::ThreadSendBufferClear();
            },
            "Network Clear Send Buffer"
        );

        auto task = server->MakeDispatchTask();
        auto dispatchThraedHandle = threadManager->Spawn("NetworkDispatch",
            engine::ThreadRole::Dispatch,
            task);
        

        bool successed = server->Start(8000);

        if(successed == false)
            return RESULT_ERROR;
   

        //  TEMP
        //  game logic thread -> 아직 테스트
        auto gameThreadHandle = threadManager->Spawn(
            "GameLogic",
            engine::ThreadRole::Game,
            [](std::stop_token st) {

                // network::SendBufferArena::ThreadSendBufferClear();

                while (st.stop_requested() == false)
                {
                    // 게임 틱
                }
            }
        );


        auto start = std::chrono::high_resolution_clock::now();

        char input;

        std::cin >> input;

        if(input == 's' || input == 'S')
        {
            threadManager->RequestStop(gameThreadHandle);
            threadManager->Join(gameThreadHandle);
            
            server->StopDispatchTask();
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








