#include "pch.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(int32_t bufferSize) : _bufferSize(bufferSize)
{
	_buffer.resize(bufferSize);
}

SendBuffer::~SendBuffer()
{
	//std::cout << "~SendBuffer" << std::endl;
}

