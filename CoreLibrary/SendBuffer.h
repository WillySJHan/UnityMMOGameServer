#pragma once

class SendBuffer : public std::enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(int32_t bufferSize);
	~SendBuffer();

	BYTE* Buffer() { return _buffer.data(); }
	int32_t WriteSize() { return _writeSize; }
	int32_t Capacity() { return static_cast<int32_t>(_buffer.size()); }

	void SetWriteSize(uint32_t size) { _writeSize = size; }
	int32_t GetBufferSize() { return _bufferSize; }

private:
	std::vector<BYTE>	_buffer;
	int32_t			_writeSize = 0;
	int32_t			_bufferSize;
};
