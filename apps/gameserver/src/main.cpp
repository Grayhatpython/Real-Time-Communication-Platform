#include "Pch.hpp"
#include "ClientSession.hpp"
#include "SessionManager.hpp"

#include "network/NetworkCore.hpp"
int main()
{
    {
 
        //  SessionFactory 
        std::function<std::shared_ptr<ClientSession>(void)> sessionFactory = []() {
            return engine::MakeShared<ClientSession>();
        };
        std::unique_ptr<SessionManager> sessionManager = std::make_unique<SessionManager>();

        sessionManager->SetSessionFactory(sessionFactory);
        std::unique_ptr<network::Server> server = std::make_unique<network::Server>(sessionManager.get());
        bool successed = server->Start(8000);

        if(successed == false)
            return RESULT_ERROR;


        auto start = std::chrono::high_resolution_clock::now();

        char input;

        std::cin >> input;

        if(input == 's' || input == 'S')
        {
            server->Stop();
        }

        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "시간 : " << duration.count() << " ms" << std::endl;
    }

    return 0;
} 








