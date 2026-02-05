#pragma once
#include "ByteSwap.hpp"

namespace engine
{
    class BinaryReader
    {
    public:
        BinaryReader() = default;
        BinaryReader(const BYTE* data, uint32 size);
        explicit BinaryReader(const std::vector<BYTE>& buffer);

    public:
        void Reset(const BYTE* data, uint32 size);
        void Reset(const std::vector<BYTE>& buffer);

    public:
        bool Read(void* out, uint32 size);
        bool Read(std::string& outStr);
        template<typename T>
        bool Read(T& out)
        {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Generic Read only supports primitive data types");

            if(_data == nullptr)
                return false;
            
            if constexpr(std::is_same<T, bool>::value)
            {
                uint8 b = 0;
                if(Read(&b, 1) == false)
                    return false;

                out =( b != 0);
                return true;
            }
            else if constexpr (std::is_enum<T>::value)
            {
                using EnumType = typename std::underlying_type<T>::type;
                EnumType enumValue{};
                if(Read<EnumType>(enumValue) == false)
                    return false;
                out = static_cast<T>(enumValue);
                return true;
            }
            else
            {
                T temp{};

                if(Read(&temp, static_cast<uint32>(sizeof(T)))== false)
                {
                    return false;     
                }

                if(STREAM_ENDIANNESS != PLATFORM_ENDIANNESS && sizeof(T) > 1)
                {
                    temp = ByteSwap<T>(temp);
                }

                out = temp;
                return true;
            }
        }

        template<typename T>
        bool Read(std::vector<T>& outVec)
        {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Generic Read only supports primitive data types");

            uint32 vectorSize = 0;
            if(Read<uint32>(vectorSize) == false)
                return false;

            outVec.clear();
            outVec.resize(vectorSize);

            if(outVec == 0)
                return true;

            if constexpr (sizeof(T) == 1 && !std::is_enum<T>::value)
            {
                return Read(outVec.data(), vectorSize);
            }
            else
            {
                for(uint32 i = 0; i < vectorSize; i++)
                {
                    if(Read<T>(outVec[i]) == false)
                        return false;
                }

                return true;
            }
        }

        bool MoveReadPos(uint32 size);

    public:
        const BYTE* GetBuffer() const { return _data; }
        uint32      GetSize() const { return _size; }
        uint32      GetReadPos() const { return _readPos; }
        uint32      Remaining() const { return (_readPos <= _size) ? (_size - _readPos) : 0; }

    private:
        const BYTE*     _data = nullptr;
        uint32          _size = 0;
        uint32          _readPos = 0;
    };
}