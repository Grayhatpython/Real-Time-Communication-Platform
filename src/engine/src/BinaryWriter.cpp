#include "engine/EnginePch.hpp"
#include "engine/BinaryWriter.hpp"

namespace engine
{
    BinaryWriter::BinaryWriter(BYTE *buffer, uint32 capacity)
        : _buffer(buffer), _capacity(capacity)
    {

    }

    void BinaryWriter::Reset(BYTE *buffer, uint32 capacity)
    {
        _buffer = buffer;
        _capacity = capacity;
        _writePos = 0;
    }

    bool BinaryWriter::Write(const void *data, uint32 size)
    {
        if(size == 0)
            return false;
        
        if(_buffer == nullptr || data == nullptr)
            return false;
        
        if(Remaining() < size)
            return false;
        
        std::memcpy(_buffer + _writePos, data, size);
        _writePos += size;

        return true;
    }

    bool BinaryWriter::Write(const std::string &str)
    {
        const uint32 strLen = static_cast<uint32>(str.size());

        if(Write<uint32>(strLen) == false)
            return false;
        
        return Write(str.data(), strLen);
    }

    
}