#pragma once

namespace servercore
{
    class BinaryWriter
    {
    public:
        BinaryWriter() = default;
        BinaryWriter(BYTE* buffer, uint32 capacity);
        ~BinaryWriter() = default;

    public:
        void Reset(BYTE* buffer, uint32 capacity);
        
        bool Write(const void* data, uint32 size);
        bool Write(const std::string& str);

        template<typename T>
        bool Write(T value)
        {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Generic Read only supports primitive data types");

            if constexpr (std::is_same<T, bool>::value)
            {
                const uint8 b = value ? 1 : 0;
                return Write(&b, 1);
            }
            else if constexpr (std::is_enum<T>::value)
            {
                using EnumType = typename std::underlying_type<T>::type;
                return Write<EnumType>(static_cast<EnumType>(value));
            }
            else 
            {
                if(STREAM_ENDIANNESS != PLATFORM_ENDIANNESS && sizeof(T) > 1)
                {
                    value = ByteSwap<T>(value);
                }

                NC_LOG_DEBUG("{}, {}, {}", _writePos, _capacity, sizeof(T));
                return Write(&value, static_cast<uint32>(sizeof(T)));
            }
        }

        template<typename T>
        bool Write(const std::vector<T>& vec)
        {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Generic Read only supports primitive data types");

            const uint32 vectorSize = vec.size();
            if(Write<uint32>(vectorSize) == false)
                return false;
            
            if constexpr (sizeof(T) == 1 && !std::is_enum<T>::value)
            {
                return Write(vec.data(), vectorSize);
            }
            else
            {
                for(const T& element : vec)
                {
                    if(Write<T>(element) == false)
                        return false;
                }

                return true;
            }
        }

        template<typename T>
        bool WriteAt(uint32 offset, T value)
        {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Generic Read only supports primitive data types");

            if(_buffer == nullptr)
                return false;
            if(offset > _capacity)
                return false;
            if(_capacity - offset < sizeof(T))
                return false;

            if constexpr (std::is_same<T, bool>::value)
            {
                const uint8_t b = value ? 1 : 0;
                std::memcpy(_buffer + offset, &b, 1);
                return true;
            }
            else if constexpr (std::is_enum<T>::value)
            {
                using EnumType = typename std::underlying_type<T>::type;
                return WriteAt<EnumType>(offset, static_cast<EnumType>(value));
            }
            else
            {
                if (STREAM_ENDIANNESS != PLATFORM_ENDIANNESS && sizeof(T) > 1)
                {
                    value = ByteSwap<T>(value);
                }

                std::memcpy(_buffer + offset, &value, sizeof(T));
                return true;
            }
        }

    public:
        BYTE*           GetBuffer() { return _buffer;}
        const BYTE*     GetBuffer() const { return _buffer;}

        uint32          Capacity() const { return _capacity; }
        uint32          Size() const { return _writePos; }
        uint32          GetWritePos() const { return _writePos; }
        uint32          Remaining() const { return (_writePos <= _capacity) ? ( _capacity - _writePos) : 0; }

    private:
        BYTE*       _buffer = nullptr;
        uint32      _capacity = 0;
        uint32      _writePos = 0;
    };
}