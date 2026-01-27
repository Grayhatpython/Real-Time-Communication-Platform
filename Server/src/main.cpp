#include "Pch.hpp"
#include "Session.hpp"
#include "NetworkCore.hpp"

#pragma pack(push, 1)
struct TestPacket : PacketHeader
{
    uint64 playerId;
    uint64 playerMp;
};
#pragma pack(pop)

class ClientSession : public servercore::Session
{
public:
    virtual void OnConnected() override
    {
        std::cout << "Client to Connected" << std::endl;
    }

    virtual void OnDisconnected() override
    {

    }

    virtual void OnRecv(BYTE* buffer, int32 numOfBytes) override
    {
        TestPacket* testPacket = reinterpret_cast<TestPacket*>(buffer);

        std::cout << testPacket->playerId << " " << testPacket->playerMp << " " << testPacket->size << " " << std::endl;
    }

    virtual void OnSend() override
    {

    }

private:

};

int main()
{
    {
        //  SessionFactory 
        std::function<std::shared_ptr<ClientSession>(void)> sessionFactory = []() {
            return servercore::MakeShared<ClientSession>();
        };

        std::unique_ptr<servercore::Server> server = std::make_unique<servercore::Server>(sessionFactory);
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








