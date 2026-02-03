#include "Pch.hpp"
#include "Session.hpp"
#include "NetworkCore.hpp"

enum class PacketId : uint16_t
{
    Stat = 200,
};

#pragma pack(push, 1)
struct StatPacket : PacketHeader
{
    uint64 playerId;
    uint32 playerHp;
    uint32 playerMp;

    bool Serialize(servercore::BinaryWriter& bw) const
    {
        return bw.Write<uint64>(playerId)
            && bw.Write<uint32>(playerHp)
            && bw.Write<uint32>(playerMp);
    }

    bool DeSerialize(servercore::BinaryReader& br)
    {
        return br.Read(playerId)
            && br.Read(playerHp)
            && br.Read(playerMp);
    }
};
#pragma pack(pop)

class ClientSession : public servercore::Session
{
public:
    virtual void OnConnected() override
    {
        
    }

    virtual void OnDisconnected() override
    {

    }

    virtual void OnRecv(BYTE* buffer, int32 numOfBytes) override
    {
        servercore::BinaryReader br(buffer, numOfBytes);
        br.MoveReadPos(sizeof(PacketHeader));

        StatPacket statPacket;
        statPacket.DeSerialize(br);

        std::cout << statPacket.playerId << " " << statPacket.playerMp << " " << std::endl;
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








