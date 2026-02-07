#include "network/NetworkPch.hpp"
#include "network/NetworkDispatcher.hpp"
#include "network/NetworkUtils.hpp"
#include "network/Session.hpp"
#include "network/Acceptor.hpp"
#include "network/SessionRegistry.hpp"
#include "network/NetworkDispatcher.hpp"

namespace network
{
    EpollDispatcher::EpollDispatcher()
    {
    }
    
    EpollDispatcher::~EpollDispatcher() 
    {
     
    }

    bool EpollDispatcher::Initialize()
    {
        _epollFd = ::epoll_create1(EPOLL_CLOEXEC);
        if(_epollFd == INVALID_EPOLL_FD_VALUE)
            return false;

        if(RegisterWakeupFd() == false)
            return false;

        _epollEvents.resize(S_DEFALUT_EPOLL_EVENT_SIZE);
        
        _sessionRegistry->SetShardWakeup(_shardId, [this](){
            this->PostWakeup();
        });

        return true;
    }

    void EpollDispatcher::Stop()
    {
        _running.store(false, std::memory_order_release);

        PostWakeup();

        EN_LOG_INFO("Epoll Dispatcher Stopped");
    }

    void EpollDispatcher::Run(std::stop_token st)
    {
        while (_running.load(std::memory_order_acquire) && st.stop_requested() == false)
        {
            Dispatch();
        }
    }

    bool EpollDispatcher::RegisterWakeupFd()
    {
        _wakeupFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (_wakeupFd == INVALID_EVENT_FD_VALUE) 
            return false;

        epoll_event epollEvent{};
        // counter > 0 이면 readable
        epollEvent.events = EPOLLIN;                  
        epollEvent.data.fd = _wakeupFd;

        if(::epoll_ctl(_epollFd, EPOLL_CTL_ADD, _wakeupFd, &epollEvent) == RESULT_ERROR)
            return false;

        return true;
    }

    bool EpollDispatcher::Register(std::shared_ptr<INetworkObject> networkObject)
    {
        uint64 sessionId = 0;

        if(networkObject == nullptr)
            return false;

        struct epoll_event epollEv;
        epollEv.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP;
        epollEv.data.ptr = networkObject.get();

        //  현재는 Session 타입만 와서 필요없긴한데..
        if(networkObject->GetNetworkObjectType() == NetworkObjectType::Session)
        {   
            auto session = std::static_pointer_cast<Session>(networkObject);
            sessionId = session->GetSessionId();    
            epollEv.data.u64 = sessionId;
        }

        if(::epoll_ctl(_epollFd, EPOLL_CTL_ADD, networkObject->GetSocketFd(), &epollEv) == RESULT_ERROR)
            return false;

        return true;
    }

    DispatchResult  EpollDispatcher::Dispatch(uint32 timeoutMs)
    {
        int32 numOfEvents = ::epoll_wait(_epollFd, _epollEvents.data(), static_cast<int32>(_epollEvents.size()), timeoutMs); 

        if(numOfEvents == 0)
            return DispatchResult::Timeout;
        
        if(numOfEvents < 0)
        {
            if(errno == EINTR)
                return DispatchResult::Interrupted;
        }

        if(numOfEvents > 0)
        {
            for(int32 i = 0 ; i < numOfEvents; i++)
            {         
                if(_epollEvents[i].data.fd == _wakeupFd)
                {
                    ConsumeEventSignal(_wakeupFd);

                    //  TODO
                    _sessionRegistry->ExecuteCommands(_shardId, *this);

                    return DispatchResult::ExitRequested;
                }
     
                auto networkObject = static_cast<INetworkObject*>(_epollEvents[i].data.ptr);

                //  ??
                if(networkObject == nullptr)
                    continue;

                int32 events = _epollEvents[i].events;
                NetworkObjectType networkObjectType = networkObject->GetNetworkObjectType();

                //  Error Event
                if((events & (EPOLLERR | EPOLLHUP)) != 0)
                {
                    ErrorEvent errorEvent;
                    networkObject->Dispatch(&errorEvent);
                }

                //  Read Event
                if((events & EPOLLIN) != 0)
                {
                    if(networkObjectType == NetworkObjectType::Session)
                    {
                        auto session = static_cast<Session*>(networkObject);

                        if(session)
                        {
                            //  TODO
                            RecvEvent* recvEvent = engine::cnew<RecvEvent>();
                            session->Dispatch(recvEvent);
                        }
                    }
                    else
                    {
                        //  ???
                    }
                }

                if((events & EPOLLOUT) != 0)
                {
                    if (networkObjectType == NetworkObjectType::Session)
                    {
                        auto session = static_cast<Session *>(networkObject);

                        if (session)
                        {
                            if(session->GetState() == SessionState::ConnectPending)
                            {
                                //  connect
                                ConnectEvent *connectEvent = engine::cnew<ConnectEvent>();
                                session->Dispatch(connectEvent);
                            }
                            else
                            {
                                //  send enable   
                                SendEvent* sendEvent = engine::cnew<SendEvent>();
                                session->Dispatch(sendEvent);
                            }
                        }
                    }
                }
            }
        }

        if(numOfEvents == static_cast<int32>(_epollEvents.size()))
            _epollEvents.resize(_epollEvents.size() * 2);

        return DispatchResult::NetworkEventDispatched;
    }

    void EpollDispatcher::PostEventSignal(EventFd coreEventFd)
    {
        uint64 signal = 1;
        ssize_t n = ::write(coreEventFd, &signal, sizeof(signal));
        if (n < 0)
        {
            // EAGAIN이면 counter가 이미 충분히 쌓여있거나(드물지만) 일시적 상황
            // 깨우기 목적은 이미 달성됐을 가능성이 높으므로 무시해도 됨.
            if (errno != EAGAIN && errno != EINTR) 
            {
                
            }
        }
    }

    void EpollDispatcher::PostWakeup()
    {
        PostEventSignal(_wakeupFd);
    }

    void EpollDispatcher::ConsumeEventSignal(EventFd coreEventFd)
    {
            // eventfd는 내부적으로 "uint64_t 카운터"를 가진다.
            // - 다른 스레드가 write(값>0)을 하면 카운터가 그만큼 증가한다.
            // - epoll은 카운터가 0보다 크면 readable(EPOLLIN) 상태가 된다.
            //
            // read()를 8바이트(uint64_t)로 성공하면:
            // 1) 현재 누적된 카운터 값을 읽어서 가져오고
            // 2) eventfd 내부 카운터를 0으로 "리셋"한다.
            //
            // 따라서 일반적으로는 "깨울 일이 있었다"는 사실만 중요하고,
            // 카운터의 정확한 값은 보통 쓰지 않는다.
            //
            // 또한 non-blocking 모드(EFD_NONBLOCK)에서는:
            // - 읽을 값이 없으면 read()가 -1을 리턴하고 errno = EAGAIN/EWOULDBLOCK가 된다.
            // - 안전하게 상태를 정리하려면 EAGAIN까지 계속 읽는 루프를 두는 편이 정석이다.
            //
            // (특히 EPOLLET(엣지 트리거)를 쓰는 경우에는 "모두 소비"가 사실상 필수다.)

            for (;;)
            {
                uint64_t wakeupCount = 0;
                ssize_t bytesRead = ::read(coreEventFd, &wakeupCount, sizeof(wakeupCount));

                if (bytesRead == static_cast<ssize_t>(sizeof(wakeupCount)))
                {
                    // 성공적으로 8바이트를 읽었다.
                    // 이 시점에 eventfd 내부 카운터는 0으로 초기화된다.
                    //
                    // 여기서 wakeupCount는 "누적된 write 호출(또는 증가량)"의 합을 의미한다.
                    // 다만 대부분의 경우:
                    // - '몇 번 깨웠는지'보다
                    // - '깨울 일이 발생했으니 큐/상태를 처리하자'
                    // 가 목적이므로 값을 굳이 사용하지 않는다.
                    //
                    // 루프를 계속 도는 이유:
                    // - 논리적으로는 한 번 읽으면 0이 되지만,
                    // - read 직후 다른 스레드가 또 write를 해버리면 다시 카운터가 >0이 될 수 있다.
                    // - 이런 케이스까지 포함해 "현재 시점에서 남은 readable 상태를 최대한 정리"하려면
                    //   EAGAIN이 뜰 때까지 반복하는 게 가장 단순하고 안전하다.
                    continue;
                }

                if (bytesRead == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                {
                    // 더 이상 읽을 카운터가 없다(완전히 소비 완료).
                    // 즉, eventfd 카운터는 0이고 EPOLLIN 조건도 해소된 상태.
                    break;
                }

                if (bytesRead == -1 && errno == EINTR)
                {
                    // 시그널로 인해 read가 중단됐다.
                    // 이 경우는 "실패"가 아니라 "재시도"가 정석이다.
                    continue;
                }

                // 예: fd가 닫혔거나, 예상치 못한 오류.
                break;
            }
    }

    bool EpollDispatcher::EnableConnectEvent(std::shared_ptr<INetworkObject> networkObject)
    {
        return EnableEvent(networkObject);
    }

    bool EpollDispatcher::DisableConnectEvent(std::shared_ptr<INetworkObject> networkObject)
    {
        return DisableEvent(networkObject);
    }

    bool EpollDispatcher::EnableSendEvent(std::shared_ptr<INetworkObject> networkObject)
    {
        return EnableEvent(networkObject);
    }

    bool EpollDispatcher::DisableSendEvent(std::shared_ptr<INetworkObject> networkObject)
    {
        return DisableEvent(networkObject);
    }

    // 지금은 더 못 보냄 → 나중에 "쓸 수 있게 되면" 알려달라
    bool EpollDispatcher::EnableEvent(const std::shared_ptr<INetworkObject>& networkObject)
    {
        if(networkObject == nullptr)
            return false;

        struct epoll_event epollEv;
        epollEv.events |= EPOLLOUT;
        epollEv.data.ptr = networkObject.get();

        if(::epoll_ctl(_epollFd, EPOLL_CTL_MOD, networkObject->GetSocketFd(), &epollEv) == RESULT_ERROR)
            return false;

        return true;
    }

    // sendQueue가 비었다 = 더 보낼 게 없다
    bool EpollDispatcher::DisableEvent(const std::shared_ptr<INetworkObject>& networkObject)
    {
        if(networkObject == nullptr)
            return false;

        struct epoll_event epollEv;
        epollEv.events  &= ~EPOLLOUT;
        epollEv.data.ptr = networkObject.get();

        if(::epoll_ctl(_epollFd, EPOLL_CTL_MOD, networkObject->GetSocketFd(), &epollEv) == RESULT_ERROR)
            return false;

        return true;
    }

    bool EpollDispatcher::UnRegister(std::shared_ptr<INetworkObject> networkObject)
    {   
        //  networkObject ( Session, Acceptor ) Event 해제
        if (::epoll_ctl(_epollFd, EPOLL_CTL_DEL, networkObject->GetSocketFd(), nullptr) == RESULT_ERROR) 
            return false;

        return true;
    }
}