#include "network/NetworkPch.hpp"
#include "network/SendBufferPool.hpp"
#include "network/SendBuffer.hpp"

namespace network
{
    bool SendBufferPool::S_SendBufferDeleterRelease = false;

    thread_local std::shared_ptr<SendBuffer> SendBufferArena::S_LCurrentSendBuffer;
    std::shared_ptr<SendBufferPool>          SendBufferArena::S_SendBufferPool = std::make_shared<SendBufferPool>();

    SendBufferPool::SendBufferPool(int32 poolSize)
    {
        std::lock_guard<std::mutex> lock(_lock);
        for (auto i = 0; i < poolSize; i++)
            _pool.push(new SendBuffer());
    }

    std::shared_ptr<SendBuffer> SendBufferPool::Pop()
    {
        SendBuffer* sendBuffer = nullptr;

        {
            std::lock_guard<std::mutex> lock(_lock);
            if (_pool.empty() == false)
            {
                sendBuffer = _pool.front();
                _pool.pop();
            }
        }

        if (sendBuffer == nullptr)
            sendBuffer = new SendBuffer();

        // 여기서 “항상” 동일한 deleter 부착
        return std::shared_ptr<SendBuffer>(sendBuffer, &SendBufferPool::ReturnSendBufferToPool);
    }

    void SendBufferPool::Push(SendBuffer* sendBuffer)
    {
        sendBuffer->Reset();
        
        std::lock_guard<std::mutex> lock(_lock);
        {
            _pool.push(sendBuffer);
        }
    }

    //  TEMP
    void SendBufferPool::ReturnSendBufferToPool(SendBuffer* sendBuffer)
    {
        if(S_SendBufferDeleterRelease == false)
            SendBufferArena::GetSendBufferPool()->Push(sendBuffer);
        else
            delete sendBuffer;
    }

    void SendBufferPool::ReleaseSendBufferDeleter()
    {
        S_SendBufferDeleterRelease = true;
    }

    void SendBufferPool::PoolClear()
    {
        std::lock_guard<std::mutex> lock(_lock);
        while(_pool.empty() == false)
        {
            auto sendBuffer = _pool.front();
            if(sendBuffer)
            {
                delete sendBuffer;
                sendBuffer = nullptr;
            }

            _pool.pop();
        }

        EN_LOG_DEBUG("SendBuffer Pool Clear");
    }

    std::shared_ptr<SendBufferSegment> SendBufferArena::Allocate(int32 size)
    {
        if (S_LCurrentSendBuffer == nullptr || S_LCurrentSendBuffer->CanUseSize(size) == false)
            SwapSendBuffer();

        BYTE* allocatedPtr = S_LCurrentSendBuffer->Allocate(size);
        return engine::MakeShared<SendBufferSegment>( allocatedPtr, allocatedPtr != nullptr, S_LCurrentSendBuffer );
    }

    int32 SendBufferArena::GetCurrentSendBufferRemainSize()
    {
        return S_LCurrentSendBuffer ? S_LCurrentSendBuffer->GetRemainSize() : 0;
    }

    int32 SendBufferArena::GetCurrentSendBufferUsedSize()
    {
        return S_LCurrentSendBuffer ? S_LCurrentSendBuffer->GetUsedSize() : 0;
    }

    int32 SendBufferArena::GetCurrentSendBufferRefCount()
    {
        return S_LCurrentSendBuffer ? S_LCurrentSendBuffer.use_count() : 0;
    }

    void SendBufferArena::ThreadSendBufferClear() 
    {
		EN_LOG_INFO("Thread Send Buffer Clear");
        S_LCurrentSendBuffer.reset(); 
    }

    void SendBufferArena::SendBufferPoolClear() 
    { 
        S_SendBufferPool->ReleaseSendBufferDeleter();
        S_SendBufferPool->PoolClear();
        S_SendBufferPool.reset(); 
    }

    void SendBufferArena::SwapSendBuffer()
    {
        S_LCurrentSendBuffer = S_SendBufferPool->Pop();
    }

}
