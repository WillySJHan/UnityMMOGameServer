#include "pch.h"
#include "IOCPBase.h"
#include "IOCPEvent.h"

IOCPBase::IOCPBase()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	SJ_ASSERT(_iocpHandle != INVALID_HANDLE_VALUE);
}

IOCPBase::~IOCPBase()
{
	::CloseHandle(_iocpHandle);
}

bool IOCPBase::Register(std::shared_ptr<IOCPObject> iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, /*key*/0, 0);
}

bool IOCPBase::Dispatch(uint32_t timeoutMs)
{
	DWORD bytes = 0;
	ULONG_PTR key = 0;
	IOCPEvent* iocpEvent = nullptr;

	if (::GetQueuedCompletionStatus(_iocpHandle, OUT &bytes, OUT &key, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		std::shared_ptr<IOCPObject> iocpObject = iocpEvent->owner;
		iocpObject->Dispatch(iocpEvent, bytes);
	}
	else
	{
		int32_t errCode = ::WSAGetLastError();
		switch (errCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			// TODO : ·Î±× Âï±â
			std::shared_ptr<IOCPObject> iocpObject = iocpEvent->owner;
			iocpObject->Dispatch(iocpEvent, bytes);
			break;
		}
	}

	return true;
}
