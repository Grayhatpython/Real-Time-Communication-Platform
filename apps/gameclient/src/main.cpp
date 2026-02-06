#include "Pch.hpp"
#include "ServerSession.hpp"
#include "SessionManager.hpp"

#include "network/NetworkCore.hpp"
int main()
{
    {
 
        //  SessionFactory 
        std::function<std::shared_ptr<ServerSession>(void)> sessionFactory = []() {
            return engine::MakeShared<ServerSession>();
        };

        std::unique_ptr<SessionManager> sessionManager = std::make_unique<SessionManager>();

        sessionManager->SetSessionFactory(sessionFactory);
        std::unique_ptr<network::Client> client = std::make_unique<network::Client>(sessionManager.get());
        network::NetworkAddress targetAddress{"127.0.0.1", 8000};
        bool successed = client->Connect(targetAddress);

        if(successed == false)
            return RESULT_ERROR;


        auto start = std::chrono::high_resolution_clock::now();

        char input;

        std::cin >> input;

        if(input == 'c' || input == 'C')
        {
            client->Stop();
        }

        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "시간 : " << duration.count() << " ms" << std::endl;
    }

    return 0;
} 








