#include "Pch.hpp"
#include "SendBufferPool.hpp"
#include "SendBuffer.hpp"

namespace servercore
{
    bool SendBufferPool::S_SendBufferDeleterRelease = false;

    thread_local std::shared_ptr<SendBuffer> SendBufferArena::S_LCurrentSendBuffer;
    std::shared_ptr<SendBufferPool>          SendBufferArena::S_SendBufferPool = std::make_shared<SendBufferPool>();

    SendBufferPool::SendBufferPool(int32 poolSize)
    {
        std::lock_guard<std::mutex> lock(_lock);
        for (auto i = 0; i < poolSize; i++)
            _pool.push(std::shared_ptr<SendBuffer>(new SendBuffer(), Push));
    }

    std::shared_ptr<SendBuffer> SendBufferPool::Pop()
    {
        {
            std::lock_guard<std::mutex> lock(_lock);
            if (_pool.empty() == false)
            {
                auto sendBuffer = _pool.front();
                _pool.pop();
                sendBuffer->Reset();
                return sendBuffer;
            }
        }

        return std::shared_ptr<SendBuffer>(new SendBuffer(), Push);
    }

    //  TEMP
    void SendBufferPool::Push(SendBuffer* sendBuffer)
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

    std::shared_ptr<SendBufferSegment> SendBufferArena::Allocate(int32 size)
    {
        if (S_LCurrentSendBuffer == nullptr || S_LCurrentSendBuffer->CanUseSize(size) == false)
            SwapSendBuffer();

        BYTE* allocatedPtr = S_LCurrentSendBuffer->Allocate(size);
        return MakeShared<SendBufferSegment>( allocatedPtr, allocatedPtr != nullptr, S_LCurrentSendBuffer );
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
        S_LCurrentSendBuffer.reset(); 
    }

    void SendBufferArena::SendBufferPoolClear() 
    { 
        S_SendBufferPool->ReleaseSendBufferDeleter();
        S_SendBufferPool.reset(); 
    }

    void SendBufferArena::SwapSendBuffer()
    {
        S_LCurrentSendBuffer = S_SendBufferPool->Pop();
    }

}
