#pragma once

class RecvBuffer
{
	enum { BUFFER_COUNT = 10 };

public:
	RecvBuffer(int32_t bufferSize);
	~RecvBuffer();

	void			Clean();
	bool			OnRead(int32_t bytes);
	bool			OnWrite(int32_t bytes);

	BYTE*			ReadPos() { return &_buffer[_readPos]; }
	BYTE*			WritePos() { return &_buffer[_writePos]; }
	int32_t			DataSize() { return _writePos - _readPos; }
	int32_t			FreeSize() { return _capacity - _writePos; }

private:
	int32_t				_capacity = 0;
	int32_t				_bufferSize = 0;
	int32_t				_readPos = 0;
	int32_t				_writePos = 0;
	std::vector<BYTE>	_buffer;
};
