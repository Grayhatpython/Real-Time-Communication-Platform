#include "network/NetworkPch.hpp"
#include "network/StreamBuffer.hpp"

namespace network
{
	StreamBuffer::StreamBuffer(int32 capacity)
		: _capacity(capacity), _buffer(capacity)
	{

	}

	bool StreamBuffer::OnRead(int32 numOfBytes)
	{
		if (numOfBytes > GetReadableSize())
			return false;

		_readPos += numOfBytes;
		return true;
	}

	bool StreamBuffer::OnWrite(int32 numOfBytes)
	{
		if (numOfBytes > GetWriteableSize())
			return false;

		_writePos += numOfBytes;
		return true;
	}

	void StreamBuffer::Clean()
	{
		const int32 readableSize = GetReadableSize();

        //  읽을 데이터가 없으면 초기화
		if (readableSize == 0)
		{
			_readPos = _writePos = 0;
		}
		else
		{
            // 읽을 데이터가 남아있으면, 해당 데이터를 버퍼의 맨 앞으로 복사
			::memmove(_buffer.data(), _buffer.data() + _readPos, readableSize);
			_readPos = 0;
			_writePos = readableSize;
		}
	}
}