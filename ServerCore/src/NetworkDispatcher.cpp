#include "Pch.hpp"
#include "NetworkDispatcher.hpp"
#include "NetworkUtils.hpp"
#include "Session.hpp"
#include "Acceptor.hpp"
#include "SessionManager.hpp"

namespace servercore
{
    EpollDispatcher::EpollDispatcher()
    {
    }
    
    EpollDispatcher::~EpollDispatcher() 
    {
        if(_controlEvents.shutdownFd != INVALID_EVENT_FD_VALUE)
        {
            ::close(_controlEvents.shutdownFd);
            _controlEvents.shutdownFd = INVALID_EVENT_FD_VALUE;
        }

        if(_controlEvents.removeSessionFd != INVALID_EVENT_FD_VALUE)
        {
            ::close(_controlEvents.removeSessionFd);
            _controlEvents.removeSessionFd = INVALID_EVENT_FD_VALUE;
        }

        if(_epollFd != INVALID_EPOLL_FD_VALUE)
        {
            ::close(_epollFd);
            _epollFd = INVALID_EPOLL_FD_VALUE;
        }
    }

    bool EpollDispatcher::Initialize()
    {
        _epollFd = ::epoll_create1(0);
        if(_epollFd == INVALID_EPOLL_FD_VALUE)
            return false;

        if(RegisterShutdownFd() == false || RegisterRemoveSessionFd() == false)
            return false;

        _epollEvents.resize(S_DEFALUT_EPOLL_EVENT_SIZE);

        return true;
    }
    
    bool EpollDispatcher::RegisterShutdownFd()
    {
        _controlEvents.shutdownFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (_controlEvents.shutdownFd == RESULT_ERROR) 
            return false;

        epoll_event epollEvent{};
        // counter > 0 이면 readable
        epollEvent.events = EPOLLIN;                 
        epollEvent.data.fd = _controlEvents.shutdownFd;

        if(::epoll_ctl(_epollFd, EPOLL_CTL_ADD,_controlEvents.shutdownFd , &epollEvent) == RESULT_ERROR)
            return false;

        return true;
    }

    bool EpollDispatcher::RegisterRemoveSessionFd()
    {
        _controlEvents.removeSessionFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (_controlEvents.removeSessionFd == RESULT_ERROR) 
            return false;

        epoll_event epollEvent{};
        // counter > 0 이면 readable
        epollEvent.events = EPOLLIN;                 
        epollEvent.data.fd = _controlEvents.removeSessionFd;

        if(::epoll_ctl(_epollFd, EPOLL_CTL_ADD,_controlEvents.removeSessionFd , &epollEvent) == RESULT_ERROR)
            return false;

        return true;
    }

    bool EpollDispatcher::Register(std::shared_ptr<INetworkObject> networkObject)
    {
        if(networkObject == nullptr)
            return false;

        //  networkobject ( Session , Acceptor ) Event 등록
        struct epoll_event epollEv;
        epollEv.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP;
        epollEv.data.ptr = networkObject.get();

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
                if(_epollEvents[i].data.fd == _controlEvents.shutdownFd)
                {
                    return DispatchResult::ExitRequested;
                }

                if(_epollEvents[i].data.fd == _controlEvents.removeSessionFd)
                {
                    //  TODO
                    GSessionManager->RemoveSession();
                    return DispatchResult::ControlEventDispatched;
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
                    int32 error = 0;
                    socklen_t len = sizeof(error);
                    ::getsockopt(networkObject->GetSocketFd(), SOL_SOCKET, SO_ERROR, &error, &len);

                    ErrorEvent errorEvent;

                    networkObject->Dispatch(&errorEvent);
                }

                //  Read Event
                if((events & EPOLLIN) != 0)
                {
                    if(networkObjectType == NetworkObjectType::Acceptor)
                    {
                        auto acceptor = static_cast<Acceptor*>(networkObject);

                        if(acceptor)
                        {
                            //  TODO
                            AcceptEvent* acceptEvent = cnew<AcceptEvent>();
                            acceptor->Dispatch(acceptEvent);
                        }
                    }   
                    else if(networkObjectType == NetworkObjectType::Session)
                    {
                        auto session = static_cast<Session*>(networkObject);

                        if(session)
                        {
                            //  TODO
                            RecvEvent* recvEvent = cnew<RecvEvent>();
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
                                ConnectEvent *connectEvent = cnew<ConnectEvent>();
                                session->Dispatch(connectEvent);
                            }
                            else
                            {
                                //  send enable   
                                
                            }
                        }
                    }
                    else if(networkObjectType == NetworkObjectType::Acceptor)
                    {
                        ;
                    }
                    {
                        ;
                    }
                }
            }
        }

        if(numOfEvents == static_cast<int32>(_epollEvents.size()))
            _epollEvents.resize(_epollEvents.size() * 2);

        return DispatchResult::ControlEventDispatched;
    }

    void EpollDispatcher::PostShutdownEvent()
    {
        PostEventSignal(_controlEvents.shutdownFd);
    }

    void EpollDispatcher::PostSessionRemoveEvent()
    {
        PostEventSignal(_controlEvents.removeSessionFd);   
    }

    void EpollDispatcher::PostEventSignal(EventFd controlEvent)
    {
        uint64 signal = 1;
        ssize_t n = ::write(controlEvent, &signal, sizeof(signal));
        if (n < 0)
        {
            // EAGAIN이면 counter가 이미 충분히 쌓여있거나(드물지만) 일시적 상황
            // 깨우기 목적은 이미 달성됐을 가능성이 높으므로 무시해도 됨.
            if (errno != EAGAIN && errno != EINTR) 
            {
                
            }
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