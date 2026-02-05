#pragma once

#define STREAM_ENDIANNESS 1 // big - Write

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define PLATFORM_ENDIANNESS 0   //  little
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define PLATFORM_ENDIANNESS 1   //  big
    #else
        #error "Unknown platform endianness"
    #endif
#else
    #error "Endianness macros not available"
#endif

#include <pthread.h>

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
#include "CommonType.hpp"
#include "ThreadLocal.hpp"
#include "GlobalContext.hpp"
#include "Logger.hpp"



