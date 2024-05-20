#include "pch.h"
#include "Session.h"
#include "SocketManager.h"
#include "Service.h"

Session::Session() : _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketManager::CreateSocket();
}

Session::~Session()
{
	SocketManager::Close(_socket);
	std::cout << "~ Session" << std::endl;
}

void Session::Send(std::shared_ptr<SendBuffer> sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool registerSend = false;

	// 현재 RegisterSend가 걸리지 않은 상태라면, 걸어준다
	{
		std::lock_guard<std::recursive_mutex> lock(_rMutex);

		_sendQueue.push(sendBuffer);

		/*if (_sendRegistered == false)
		{
			_sendRegistered = true;
			RegisterSend();
		}*/

		if (_sendRegistered.exchange(true) == false)
			registerSend = true;
	}

	if (registerSend)
		RegisterSend();
}

bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	std::wcout << "Disconnect : " << cause << std::endl;

	RegisterDisconnect();
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IOCPEvent* iocpEvent, int32_t bytes)
{
	switch (iocpEvent->eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Disconnect:
		ProcessDisconnect();
		break;
	case EventType::Recv:
		ProcessRecv(bytes);
		break;
	case EventType::Send:
		ProcessSend(bytes);
		break;
	default:
		break;
	}
}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (GetService()->GetServiceType() != ServiceType::Client)
		return false;

	if (SocketManager::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketManager::BindAnyAddress(_socket, 0) == false)
		return false;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this(); // REF++

	DWORD bytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetSocketAddress().GetSocketAddress();
	if (false == SocketManager::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr, 0, &bytes, &_connectEvent))
	{
		int32_t errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_connectEvent.owner = nullptr; // REF--
			return false;
		}
	}

	return true;
}

bool Session::RegisterDisconnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this(); // REF++

	if (false == SocketManager::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int32_t errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_disconnectEvent.owner = nullptr; // REF--
			return false;
		}
	}

	return true;
}

void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	_recvEvent.Init();
	_recvEvent.owner = shared_from_this(); // REF++

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

	DWORD bytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT &bytes, OUT &flags, &_recvEvent, nullptr))
	{
		int32_t errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_recvEvent.owner = nullptr; // REF--
		}
	}
}

void Session::RegisterSend()
{
	if (IsConnected() == false)
		return;

	_sendEvent.Init();
	_sendEvent.owner = shared_from_this(); // REF++

	// 보낼 데이터를 sendEvent에 등록
	{
		std::lock_guard<std::recursive_mutex> lock(_rMutex);

		int32_t writeSize = 0;
		while (_sendQueue.empty() == false)
		{
			std::shared_ptr<SendBuffer> sendBuffer = _sendQueue.front();

			writeSize += sendBuffer->WriteSize();
			// TODO : 예외 체크

			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer);
		}
	}

	// Scatter-Gather (흩어져 있는 데이터들을 모아서 한 방에 보낸다)
	std::vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendBuffers.size());
	for (auto sendBuffer : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD bytes = 0;
	if (SOCKET_ERROR == ::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT &bytes, 0, &_sendEvent, nullptr))
	{
		int32_t errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_sendEvent.owner = nullptr; // REF--
			// TODO SJ
			_sendEvent.sendBuffers.clear(); // REF--
			_sendRegistered.store(false);
		}
	}
}

void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr; // REF--

	_connected.store(true);

	GetService()->AddSession(GetSessionPtr());

	OnConnected();

	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.owner = nullptr; // REF--

	OnDisconnected(); // 컨텐츠 코드에서 오버로딩
	GetService()->ReleaseSession(GetSessionPtr());
}

void Session::ProcessRecv(int32_t bytes)
{
	_recvEvent.owner = nullptr; // REF--

	if (bytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	if (_recvBuffer.OnWrite(bytes) == false)
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	int32_t dataSize = _recvBuffer.DataSize();
	int32_t processLen = OnRecv(_recvBuffer.ReadPos(), dataSize); 
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	_recvBuffer.Clean();

	RegisterRecv();
}

void Session::ProcessSend(int32_t bytes)
{
	_sendEvent.owner = nullptr; // REF--
	_sendEvent.sendBuffers.clear(); // REF--

	if (bytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}

	OnSend(bytes);

	bool registerSend = false;
	
	{
		std::lock_guard<std::recursive_mutex> lock(_rMutex);

		if (_sendQueue.empty())
			_sendRegistered.store(false);
		else
		{
			registerSend = true;
			//RegisterSend();
		}
	}
	if (registerSend)
		RegisterSend();
}

void Session::HandleError(int32_t errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
		break;
	default:
		std::cout << "Handle Error : " << errorCode << std::endl;
		break;
	}
}


ProtocolSession::ProtocolSession()
{
}

ProtocolSession::~ProtocolSession()
{
}


int32_t ProtocolSession::OnRecv(BYTE* buffer, int32_t len)
{
	int32_t processLen = 0;

	while (true)
	{
		int32_t dataSize = len - processLen;
		// 최소한 헤더는 파싱할 수 있어야 한다
		if (dataSize < sizeof(ProtocolHeader))
			break;

		ProtocolHeader header = *(reinterpret_cast<ProtocolHeader*>(&buffer[processLen]));
		// 헤더에 기록된 패킷 크기를 파싱할 수 있어야 한다
		if (dataSize < header.size)
			break;

		// 패킷 조립 성공
		OnRecvProtocol(&buffer[processLen], header.size);

		processLen += header.size;
	}

	return processLen;
}

