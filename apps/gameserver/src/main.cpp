#include "Pch.h"
#include <chrono>
#include <thread>
#include <memory>
#include <functional>

#include "ClientSession.h"

#include "engine/MemoryPool.h"
#include "engine/GlobalContext.h"
#include "engine/ThreadManager.h"

#include "network/NetworkCore.h"
#include "network/SendBufferPool.h"
#include "network/SessionRegistry.h"

#include "World.h"

int main()
{
    {
        engine::GlobalContext::GetInstance().Initialize();
        World* world = new World();
        
        engine::ThreadManager* threadManager = engine::GlobalContext::GetInstance().GetThreadManager();

        //  SessionFactory 
        std::function<std::shared_ptr<network::Session>(void)> sessionFactory = [world]() {
            return engine::MakeShared<ClientSession>(world);
        };
        
        std::unique_ptr<network::SessionRegistry> sessionRegistry = std::make_unique<network::SessionRegistry>(2, sessionFactory);

        network::Server* server = new network::Server(sessionRegistry.get(), 8000, 2);
        if(server->Initialize() == false)
            return RESULT_ERROR;

        threadManager->RegisterExitCallback(
            engine::ThreadRole::Dispatch,
            [](){
                network::SendBufferArena::ThreadSendBufferClear();
            },
            "Network Clear Send Buffer"
        );

        for (int i = 0; i < server->GetDispatchThreadCount(); ++i) 
        {
            threadManager->Spawn("Network Dispatch " + std::to_string(i), 
                engine::ThreadRole::Dispatch,
                server->MakeDispatchTask(i));
        }
    
        threadManager->Spawn("Network Accept", 
            engine::ThreadRole::Dispatch,
            server->MakeAcceptTask()
        );

        /*
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
        */

        auto start = std::chrono::high_resolution_clock::now();

        char input;

        std::cin >> input;

        if(input == 's' || input == 'S')
        {
            server->Stop();
            threadManager->StopAllAndJoin();
            sessionRegistry.reset();

            if(world)
            {
                delete world;
                world = nullptr;
            }

            engine::GlobalContext::GetInstance().Clear();
        }

        if(server)
        {
            delete server;
            server = nullptr;
        }
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "시간 : " << duration.count() << " ms" << std::endl;
    }

    return 0;
} 








