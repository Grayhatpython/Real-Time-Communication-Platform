#pragma once 

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

