#include "Pch.hpp"
#include "NetworkDispatcher.hpp"
#include "NetworkUtils.hpp"
#include "Session.hpp"
#include "Acceptor.hpp"

namespace servercore
{
    EpollDispatcher::EpollDispatcher()
    {
    }
    
    EpollDispatcher::~EpollDispatcher() 
    {
        if(_epollFd)
        {
            ::close(_epollFd);
            _epollFd = INVALID_EPOLL_FD_VALUE;
        }

        for(int i = 0; i < 2; i++)
        {
            if(_exitSignalEventPipe[i] >= 0)
            {
                ::close(_exitSignalEventPipe[i]);
                _exitSignalEventPipe[i] = INVALID_FILE_DESCRIPTOR_VALUE;
            }
        }
    }

    bool EpollDispatcher::Initialize()
    {
        _epollFd = ::epoll_create1(0);
        if(_epollFd == INVALID_EPOLL_FD_VALUE)
            return false;

        //  pipe() 함수를 호출하여 두 개의 파일 디스크립터를 가진 파이프를 생성
        //  파이프는 단방향 통신을 위한 커널 객체
        //  _exitSignalEventPipe[0]은 파이프의 읽기 끝(read end)이고, _exitSignalEventPipe[1]은 파이프의 쓰기 끝(write end)
        //  디스패처 스레드를 안전하게 종료하기 위한 "종료 시그널" 메커니즘으로 사용
        //  다른 스레드에서 _exitSignalEventPipe[1]에 데이터를 쓰면, epoll_wait에서 _exitSignalEventPipe[0]에 이벤트가 발생하여 디스패처 스레드가 깨어나 종료 처리
        if(::pipe(_exitSignalEventPipe) == RESULT_ERROR)
            return false;

        //  read end / write end 둘 다 non-blocking
        for(int i = 0; i < 2; i++)
        {
            if(NetworkUtils::SetNonBlocking(_exitSignalEventPipe[i]) == false)
                return false;
        }

        //  pipe의 read end에 데이터가 들어오면(=누가 write 했다면) epoll이 깨워라
        struct epoll_event epollEv;
        epollEv.events = EPOLLIN;
        epollEv.data.fd = _exitSignalEventPipe[0];

        //  epoll_wait -> exitSignalEventPipe[0]에 데이터가 쓰여지면 이벤트를 감지
        if(::epoll_ctl(_epollFd, EPOLL_CTL_ADD,_exitSignalEventPipe[0], &epollEv) == RESULT_ERROR)
            return false;

        _epollEvents.resize(S_DEFALUT_EPOLL_EVENT_SIZE);

        return true;
    }
    
    bool EpollDispatcher::Register(std::shared_ptr<INetworkObject> networkObject)
    {
        if(networkObject == nullptr)
            return false;

        //  networkobject ( Session , Acceptor ) Event 등록
        struct epoll_event epollEv;
        epollEv.events = EPOLLIN | EPOLLET;
        epollEv.data.ptr = networkObject.get();

        if(::epoll_ctl(_epollFd, EPOLL_CTL_ADD, networkObject->GetSocketFd(), &epollEv) == RESULT_ERROR)
            return false;

        return true;
    }

    DispatchResult  EpollDispatcher::Dispatch(uint32 timeoutMs = TIMEOUT_INFINITE)
    {
        int32 numOfEvents = ::epoll_wait(_epollFd, _epollEvents.data(), static_cast<int32>(_epollEvents.size()), timeoutMs); 

        if(numOfEvents < 0)
        {
            if(errno == EINTR)
                return DispatchResult::Timeout;
            else
                return DispatchResult::CriticalError;
        }
        else if(numOfEvents == 0)
            return DispatchResult::Timeout;
        else
        {
            for(int32 i = 0 ; i < numOfEvents; i++)
            {
                if(_epollEvents[i].data.fd == _exitSignalEventPipe[0])
                {
                    char buffer[0];
                    ::read(_exitSignalEventPipe[0], buffer, sizeof(buffer));
                    return DispatchResult::Exit;
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

                    networkObject->Dispatch(&errorEvent, false, error);
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
                            acceptor->Dispatch(acceptEvent, true, static_cast<int32>(ErrorCode::Success));
                        }
                    }   
                    else if(networkObjectType == NetworkObjectType::Session)
                    {
                        auto session = static_cast<Session*>(networkObject);

                        if(session)
                        {
                            //  TODO
                            RecvEvent* recvEvent = cnew<RecvEvent>();
                            session->Dispatch(recvEvent, true, static_cast<int32>(ErrorCode::Success));
                        }
                    }
                    else
                    {
                        //  ???
                    }
                }

                //  SendEvent   
                //  ...TODO
                if((events & EPOLLOUT) != 0)
                {
                    // if (networkObjectType == NetworkObjectType::Session)
                    // {
                    //     auto session = static_cast<Session *>(networkObject);

                    //     if (session)
                    //     {
                    //         if (session->IsConnectPending() == true)
                    //         {
                    //             //  connect
                    //             LinuxConnectEvent *connectEvent = cnew<LinuxConnectEvent>();
                    //             session->Dispatch(connectEvent, true, static_cast<int32>(ErrorCode::Success));
                    //         }
                    //         else
                    //         {
                    //             //  send
                    //             LinuxSendEvent *sendEvent = cnew<LinuxSendEvent>();
                    //             session->Dispatch(sendEvent, true, static_cast<int32>(ErrorCode::Success));
                    //         }
                    //     }
                    // }
                    // else
                    // {
                    //     ; //  ????
                    // }
                }
            }
        }

        if(numOfEvents == static_cast<int32>(_epollEvents.size()))
            _epollEvents.resize(_epollEvents.size() * 2);

        return DispatchResult::Success;
    }

    void EpollDispatcher::PostExitSignal()
    {
        char buffer[1] = {'q'};
        ::write(_exitSignalEventPipe[1], buffer, sizeof(buffer));
    }

    bool EpollDispatcher::RegisterSend(std::shared_ptr<INetworkObject> networkObject)
    {
        if(networkObject == nullptr)
            return false;

        //  networkobject ( Session , Acceptor ) Event 등록
        struct epoll_event epollEv;
        epollEv.events = EPOLLIN | EPOLLOUT | EPOLLET;
        epollEv.data.ptr = networkObject.get();

        if(::epoll_ctl(_epollFd, EPOLL_CTL_ADD, networkObject->GetSocketFd(), &epollEv) == RESULT_ERROR)
            return false;

        return true;
    }

    bool EpollDispatcher::UnRegister(std::shared_ptr<INetworkObject> networkObject)
    {   
        //  networkObject ( Session, Acceptor ) Event 해제
        if (::epoll_ctl(_epollFd, EPOLL_CTL_DEL, networkObject->GetSocketFd(), nullptr) < 0) 
            return false;

        return true;
    }
}