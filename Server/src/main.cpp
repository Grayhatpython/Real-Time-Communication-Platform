#include "Pch.hpp"


int main()
{

    std::cout << "Tset" << std::endl;
    {
           /*
        std::function<std::shared_ptr<ClientSession>()> sessionFactory = []() {
            return servercore::MakeShared<ClientSession>();
            };

        std::shared_ptr<servercore::ServerService> server = std::make_shared<servercore::ServerService>(1, sessionFactory);
        server->Start(8888);

        char input;

        while (true)
        {
            std::cin >> input;

            if (input == 'q' || input == 'Q')
                break;
        }

        server->Stop();

     
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 5; i++)
        {
            ServerCore::GThreadManager->Launch([]() {
                for (int i = 0; i < 100000; i++)
                {
                    //auto k1 = ServerCore::cnew<Knight>();
                    //ServerCore::cdelete(k1);

                    auto k1 = new Knight();
                    delete k1;
                }
                },"TestThread", false);
        }
        
        ServerCore::GThreadManager->Join();

        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "시간 : " << duration.count() << " ms" << std::endl;
        */
           
    }

    return 0;
} 








