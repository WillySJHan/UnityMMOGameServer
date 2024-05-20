#pragma once

class IOCPObject : public std::enable_shared_from_this<IOCPObject>
{
public:
	virtual HANDLE GetHandle() = 0;
	virtual void Dispatch(class IOCPEvent* iocpEvent, __int32 bytes = 0) = 0;
};

class IOCPBase
{
public:
	IOCPBase();
	~IOCPBase();
	HANDLE		GetHandle() { return _iocpHandle; }
	bool		Register(std::shared_ptr<IOCPObject> iocpObject);
	bool		Dispatch(uint32_t timeoutMs = INFINITE);

private:
	HANDLE		_iocpHandle;
};

