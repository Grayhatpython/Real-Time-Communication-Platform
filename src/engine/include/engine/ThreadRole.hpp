#pragma once

#include "CommonType.hpp"

namespace engine 
{
    enum class ThreadRole : uint32
    {
        None      = 0,
        Dispatch  = 1 << 0,  // epoll dispatcher / IO thread
        Game      = 1 << 1,  // 게임 로직 스레드
        Worker    = 1 << 2,  // 잡 워커
    };

    inline constexpr ThreadRole operator|(ThreadRole a, ThreadRole b)
    {
        return static_cast<ThreadRole>(static_cast<uint32>(a) | static_cast<uint32>(b));
    }

    inline constexpr bool HasRole(ThreadRole mask, ThreadRole role)
    {
        return (static_cast<uint32>(mask) & static_cast<uint32>(role)) != 0;
    }
} 