#include "pch.h"
#include "Listener.h"
#include "SocketManager.h"
#include "IocpEvent.h"
#include "Session.h"
#include "Service.h"

Listener::~Listener()
{
	SocketManager::Close(_socket);

	for (AcceptEvent* acceptEvent : _acceptEvents)
	{
		// TODO

		delete(acceptEvent);
	}
}

bool Listener::StartAccept(std::shared_ptr<ServerService> service)
{
	_service = service;
	if (_service == nullptr)
		return false;

	_socket = SocketManager::CreateSocket();
	if (_socket == INVALID_SOCKET)
		return false;

	if (_service->GetIocpBase()->Register(shared_from_this()) == false)
		return false;

	if (SocketManager::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketManager::SetLinger(_socket, 0, 0) == false)
		return false;

	if (SocketManager::Bind(_socket, _service->GetSocketAddress()) == false)
		return false;

	if (SocketManager::Listen(_socket) == false)
		return false;

	const int32_t acceptCount = _service->GetMaxSessionCount();
	for (int32_t i = 0; i < acceptCount; i++)
	{
		AcceptEvent* acceptEvent = new AcceptEvent();
		acceptEvent->owner = shared_from_this();
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return true;
}

void Listener::CloseSocket()
{
	SocketManager::Close(_socket);
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Listener::Dispatch(IOCPEvent* iocpEvent, int32_t Bytes)
{
	SJ_ASSERT(iocpEvent->eventType == EventType::Accept);
	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	std::shared_ptr<Session> session = _service->CreateSession(); // Register IOCP

	acceptEvent->Init();
	acceptEvent->session = session;

	DWORD bytesReceived = 0;
	if (false == SocketManager::AcceptEx(_socket, session->GetSocket(), session->_recvBuffer.WritePos(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT &bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		const int32_t errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			// 일단 다시 Accept 걸어준다
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	std::shared_ptr<Session> session = acceptEvent->session;

	if (false == SocketManager::SetUpdateAccept(session->GetSocket(), _socket))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddress;
	int32_t sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == ::getpeername(session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	session->SetSocketAddress(SocketAddress(sockAddress));

	session->ProcessConnect();

	RegisterAccept(acceptEvent);
}

