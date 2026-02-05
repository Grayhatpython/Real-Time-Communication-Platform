#pragma once

#include <pthread.h>

// Linux/Unix 소켓 프로그래밍에 필요한 표준 헤더 파일들입니다.
#include <sys/socket.h>     // 소켓 관련 기본 함수 (socket, bind, listen, accept, connect 등)
#include <sys/epoll.h>      // epoll
#include <sys/eventfd.h>
#include <netinet/in.h>     // 인터넷 주소 구조체 (sockaddr_in, sockaddr_in6) 및 상수 (IPPROTO_TCP 등)
#include <netinet/tcp.h>    // TCP_NODELAY
#include <arpa/inet.h>      // IP 주소 변환 함수 (inet_pton, inet_ntop 등)
#include <unistd.h>         // POSIX 표준 함수 (close() 등, Windows의 closesocket()에 해당)
#include <netdb.h>          // 호스트 이름/서비스 해석 함수 (getaddrinfo, freeaddrinfo 등)
#include <fcntl.h>          // Non Blocking

#include <sys/types.h>      // 다양한 데이터 타입 정의
#include <sys/syscall.h>
#include <cerrno>           // errno
#include <csignal>          // signal

#include <map>
#include <set>
#include <list>
#include <array>
#include <queue>
#include <stack>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include <mutex>
#include <thread>
#include <future>
#include <condition_variable>

#include <memory>
#include <string>
#include <iostream> 
#include <functional>

#include <cstring>
#include <cstdlib>
#include <cassert>

// 타입 정의 헤더 파일 

#include "engine/CommonType.hpp"
#include "engine/Logger.hpp"
#include "engine/Lock.hpp"

#include "network/Types.hpp"
#include "network/Enums.hpp"


//  TEMP
#pragma pack(push, 1)
struct PacketHeader
{
    uint16 size;
    uint16 id;
};
#pragma pack(pop)

