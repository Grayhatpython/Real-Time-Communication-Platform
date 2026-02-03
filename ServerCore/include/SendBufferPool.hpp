#pragma once

namespace servercore
{
    constexpr int32 DEFALUT_SEND_BUFFER_POOL_SIZE = 10;

    class SendBuffer;
    class SendBufferPool
    {
    public:
        SendBufferPool(int32 poolSize = DEFALUT_SEND_BUFFER_POOL_SIZE);

    public:
        std::shared_ptr<SendBuffer> Pop();
        void                        Push(SendBuffer* sendBuffer);

    public:
        static void ReturnSendBufferToPool(SendBuffer* sendBuffer);
        static void ReleaseSendBufferDeleter();
        void        PoolClear();

    private:
        std::queue<SendBuffer*>                     _pool;
        std::mutex                                  _lock;
        static bool                                 S_SendBufferDeleterRelease;
    };

    struct SendBufferSegment
    {
        SendBufferSegment(BYTE* ptr, bool successed, std::shared_ptr<SendBuffer> sendBuffer)
            : ptr(ptr), successed(successed), sendBuffer(sendBuffer)
        {

        }

        BYTE* ptr = nullptr;
        bool successed = false;
        std::shared_ptr<SendBuffer> sendBuffer;
    };

    class SendBufferArena
    {
    public:
        static std::shared_ptr<SendBufferSegment> Allocate(int32 size);

    public:
        static int32 GetCurrentSendBufferRemainSize();
        static int32 GetCurrentSendBufferUsedSize();
        static int32 GetCurrentSendBufferRefCount();

        static void ThreadSendBufferClear();
        static void SendBufferPoolClear();

    public:
        static std::shared_ptr<SendBufferPool> GetSendBufferPool() { return S_SendBufferPool; }

    private:
        static void SwapSendBuffer();
    
    private:
        static thread_local std::shared_ptr<SendBuffer> S_LCurrentSendBuffer;
        static std::shared_ptr<SendBufferPool>          S_SendBufferPool;
    };
}

