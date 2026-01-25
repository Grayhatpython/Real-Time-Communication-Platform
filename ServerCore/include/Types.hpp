#pragma once

#include <cstdint> 

//  TYPE
using BYTE = unsigned char; 
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;


using Fd = int;
using FileDescriptor = int;

using SocketFd = int;
using EpollFd = int;
using EventFd = int;

constexpr int TIMEOUT_INFINITE = -1;
constexpr int INVALID_FILE_DESCRIPTOR_VALUE = -1;

constexpr SocketFd  INVALID_SOCKET_FD_VALUE = INVALID_FILE_DESCRIPTOR_VALUE;
constexpr EpollFd   INVALID_EPOLL_FD_VALUE  = INVALID_FILE_DESCRIPTOR_VALUE;
constexpr EventFd   INVALID_EVENT_FD_VALUE  = INVALID_FILE_DESCRIPTOR_VALUE;

constexpr int RESULT_OK = 0;
constexpr int RESULT_ERROR = -1; 