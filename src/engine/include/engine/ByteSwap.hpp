#pragma once

#include <vector>
#include <limits>
#include <cstdint>
#include <string>
#include <cstring>     
#include <algorithm>    
#include <type_traits>

namespace engine
{
    template<typename T>
    inline T ByteSwapUnsigned(T value)
    {
        static_assert(std::is_unsigned<T>::value, "T must be unsigned.");

        if constexpr (sizeof(T) == 1)
            return value;
        else if constexpr (sizeof(T) == 2)
            return __builtin_bswap16(value);
        else if constexpr (sizeof(T) == 4)
            return __builtin_bswap32(value);
        else if constexpr (sizeof(T) == 8)
            return __builtin_bswap64(value);
        else
        {
            static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
                              "Unsupported size for byteswap.");
            return value;
        }
    }

    template<typename T>
    inline T ByteSwap(T value)
    {
        if constexpr (std::is_same<T, bool>::value)
            return value;
        else if constexpr (std::is_enum<T>::value)
        {
            using EnumType = typename std::underlying_type<T>::type;
            EnumType enumValue;

            std::memcpy(&enumValue, &value, sizeof(EnumType));
            auto swappedValue = ByteSwap<EnumType>(enumValue);
            T retValue = static_cast<T>(swappedValue);
            return retValue;
        }
        else if constexpr (std::is_integral<T>::value)
        {
            using unsignedType = typename std::make_unsigned<T>::type;
            unsignedType unsingedValue = static_cast<unsignedType>(value);
            unsignedType swappedUnsignedValue = ByteSwapUnsigned<unsignedType>(unsingedValue);
            return static_cast<T>(swappedUnsignedValue);
        }
        else if constexpr (std::is_floating_point<T>::value)
        {
            if constexpr (sizeof(T) == 4)
            {
                uint32 f = 0;
                std::memcpy(&f, &value, sizeof(f));
                f = ByteSwapUnsigned<uint32>(f);
                T ret;
                std::memcpy(&ret, &f, sizeof(ret));
                return ret;
            }
            else if constexpr (sizeof(T) == 4)
            {
                uint64 d = 0;
                std::memcpy(&d, &value, sizeof(d));
                d = ByteSwapUnsigned<uint32>(d);
                T ret;
                std::memcpy(&ret, &d, sizeof(ret));
                return ret;
            }
           else
            {
                static_assert(sizeof(T) == 4 || sizeof(T) == 8, "Only float(4) and double(8) are supported.");
                return value;
            }
        }
        else
        {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "ByteSwap supports only integral/float/enum/bool.");
            return value;
        }
    }
}