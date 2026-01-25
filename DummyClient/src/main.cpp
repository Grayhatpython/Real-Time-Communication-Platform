#include "Pch.hpp"  
#include "Session.hpp"
#include "NetworkCore.hpp"

/*
#pragma pack(push, 1)
struct TestPacket : PacketHeader
{
    uint64 playerId;
    uint64 playerMp;
};
#pragma pack(pop)


#if defined(PLATFORM_WINDOWS)
class ParallelSessionSend
{
public:
    void ProcessParallelSend(const std::vector< std::shared_ptr<servercore::Session>>& serverSesions, int32 threadCount)
    {
        std::vector<std::thread> workerThreads;
        workerThreads.reserve(threadCount);

        const auto sessionsCount = serverSesions.size();
        const auto sessionCountPerThread = sessionsCount / threadCount;
        const auto remainSessionCount = sessionsCount % threadCount;

        size_t currentIndex = 0;

        for (int32_t i = 0; i < threadCount; i++)
        {
            size_t rangeSize = sessionCountPerThread + (i < remainSessionCount ? 1 : 0);
            if (rangeSize == 0)
                continue;

            const size_t rangeStart = currentIndex;
            const size_t rangeEnd = rangeStart + rangeSize;
            currentIndex = rangeEnd;

            servercore::GThreadManager->Launch([&serverSesions, rangeStart, rangeEnd]() {
                    for (auto j = rangeStart; j < rangeEnd; j++)
                    {
                        const auto& session = serverSesions[j];
                        if (session == nullptr)
                            continue;

                        // 1. SendBuffer 할당
                        auto segment = servercore::SendBufferArena::Allocate(sizeof(TestPacket));

                        // 2. 할당 성공 여부 명시적 확인 (assert 대체)
                        if (segment->successed == false) {
                            // 실무: 메모리 풀 고갈 상황에 대한 로그 및 통계 처리
                            std::cerr << "SendBuffer allocation failed for session: " << session->GetSessionId() << ". Skipping." << std::endl;
                            continue; // 이 세션에 대한 전송은 건너뜀
                        }

                        std::cout << "Test" << std::endl;

                        // 3. 패킷 구성
                        TestPacket* testPacket = reinterpret_cast<TestPacket*>(segment->ptr);
                        testPacket->id = 3;
                        testPacket->playerId = 3;
                        testPacket->playerMp = 6;
                        testPacket->size = sizeof(TestPacket);

                        // 4. SendContext 생성 및 전송 요청
                        auto sendContext = std::make_shared<servercore::SendContext>();
                        sendContext->sendBuffer = segment->sendBuffer;
                        sendContext->wsaBuf.buf = reinterpret_cast<CHAR*>(segment->ptr);
                        sendContext->wsaBuf.len = static_cast<ULONG>(sizeof(TestPacket));

                        session->Send(sendContext);
                    }

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                });
        }
    }
};

int main()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
        std::function<std::shared_ptr<ServerSession>()> sessionFactory = []() {
            return servercore::MakeShared<ServerSession>();
            };

        std::shared_ptr<servercore::ClinetService> client = std::make_shared<servercore::ClinetService>(1, sessionFactory);      
        std::vector< std::shared_ptr<servercore::Session>> serverSessions;
        auto session = client->Connect(servercore::NetworkAddress("127.0.0.1", 8888), 1, serverSessions);

        ParallelSessionSend pss;
        pss.ProcessParallelSend(serverSessions, 1);

        while (true)
        {
            ;
        }
    }
    return 0;
}
#else
*/

class ServerSession : public servercore::Session
{
public:
    virtual void OnConnected() override
    {
        std::cout << "Server to Connected" << std::endl;
    }

    virtual void OnDisconnected() override
    {

    }

    virtual void OnRecv(BYTE* buffer, int32 numOfBytes) override
    {

    }

    virtual void OnSend() override
    {

    }

private:

};

int main(int argc, char* argv[])
{
    {
        //  SessionFactory 
        std::function<std::shared_ptr<ServerSession>(void)> sessionFactory = []() {
            return servercore::MakeShared<ServerSession>();
        };

        std::unique_ptr<servercore::Client> client = std::make_unique<servercore::Client>(sessionFactory);
        servercore::NetworkAddress targetAddress{"127.0.0.1", 8000};
        bool successed = client->Connect(targetAddress);

        if(successed == false)
            return RESULT_ERROR;


        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 1; i++)
        {
            servercore::GThreadManager->Launch([&client]() {
                    while(true)
                    {
                        auto dispatchResult = client->NetworkDispatch();

                        //  TODO
                        if(dispatchResult == servercore::DispatchResult::InvalidDispatcher || 
                            dispatchResult == servercore::DispatchResult::ExitRequested)
                        {
                            break;
                        }
                    }   
                },"Dispatch Thread");
        }
        
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
