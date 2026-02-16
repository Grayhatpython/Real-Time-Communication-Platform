#include "Pch.h"
#include <chrono>
#include <thread>
#include <memory>
#include <functional>

#include "Messenger.h"
#include "ServerSession.h"
#include "PacketHandler.h"

#include "engine/MemoryPool.h"
#include "engine/GlobalContext.h"
#include "engine/ThreadManager.h"

#include "network/NetworkCore.h"
#include "network/SendBufferPool.h"
#include "network/SessionRegistry.h"

// -------------------- main --------------------
int main()
{
    engine::GlobalContext::GetInstance().Initialize();
    engine::ThreadManager* threadManager = engine::GlobalContext::GetInstance().GetThreadManager();
    PacketHandler::RegisterPacketHandleFunc();

    //  SessionFactory 
    std::function<std::shared_ptr<network::Session>(void)> sessionFactory = []() {
        return engine::MakeShared<ServerSession>();
    };
    
    std::unique_ptr<network::SessionRegistry> sessionRegistry = std::make_unique<network::SessionRegistry>(1, sessionFactory);

    network::Client* client = new network::Client(sessionRegistry.get());
    if(client->Initialize() == false)
        return RESULT_ERROR;

    threadManager->RegisterExitCallback(
        engine::ThreadRole::Dispatch,
        [](){
            network::SendBufferArena::ThreadSendBufferClear();
        },
        "Network Clear Send Buffer"
    );

    threadManager->Spawn("Network Dispatch" , 
        engine::ThreadRole::Dispatch,
        client->MakeDispatchTask());

    
    auto targetAddress = network::NetworkAddress{"127.0.0.1", 8000};
    client->Connect(targetAddress);
    
    auto start = std::chrono::high_resolution_clock::now();
    char input;
    std::cin >> input;

    if(input == 'c' || input == 'C')
    {
        client->Stop();
        threadManager->StopAllAndJoin();
        sessionRegistry.reset();

        engine::GlobalContext::GetInstance().Clear();
    }

    if(client)
    {
        delete client;
        client = nullptr;
    }

    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> duration = end - start;

    std::cout << "시간 : " << duration.count() << " ms" << std::endl;

    return 0;
}




