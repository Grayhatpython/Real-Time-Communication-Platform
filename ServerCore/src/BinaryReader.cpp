#include "Pch.hpp"
#include "BinaryReader.hpp"

namespace servercore
{
    BinaryReader::BinaryReader(const BYTE *data, uint32 size)
        : _data(data), _size(size)
    {

    }

    BinaryReader::BinaryReader(const std::vector<BYTE> &buffer)
        : _data(buffer.empty() ? nullptr : buffer.data()), _size(static_cast<uint32>(buffer.size()))
    {

    }

    void BinaryReader::Reset(const BYTE *data, uint32 size)
    {
        _data = data;
        _size = size;
        _readPos = 0;
    }

    void BinaryReader::Reset(const std::vector<BYTE> &buffer)
    {
        _data = buffer.empty() ? nullptr : buffer.data();
        _size = static_cast<uint32>(buffer.size());
        _readPos = 0;
    }

    bool BinaryReader::Read(void *out, uint32 size)
    {
        if(size == 0)
            return false;

        if(out == nullptr || _data == nullptr)
            return false;

        if(Remaining() < size)
            return false;

        std::memcpy(out, _data + _readPos, size);
        _readPos += size;

        return true;
    }
    bool BinaryReader::Read(std::string &outStr)
    {
        uint32 strLen = 0;

        if(Read<uint32>(strLen) == false)
            return false;

        if(Remaining() < strLen)
            return false;

        outStr.clear();
        
        if(strLen == 0)
            return true;

        outStr.resize(strLen);   
        return Read(&outStr[0], strLen);
    }

    bool BinaryReader::MoveReadPos(uint32 size)
    {
        if(Remaining() < size)
            return false;

        _readPos += size;

        return true;
    }
}