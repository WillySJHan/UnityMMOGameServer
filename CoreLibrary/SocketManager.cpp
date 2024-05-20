#include "pch.h"
#include "SocketManager.h"

LPFN_CONNECTEX		SocketManager::ConnectEx = nullptr;
LPFN_DISCONNECTEX	SocketManager::DisconnectEx = nullptr;
LPFN_ACCEPTEX		SocketManager::AcceptEx = nullptr;

void SocketManager::Init()
{
	WSADATA wsaData;
	SJ_ASSERT(::WSAStartup(MAKEWORD(2, 2), OUT &wsaData) == 0);

	SOCKET dummySocket = CreateSocket();
	SJ_ASSERT(BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
	SJ_ASSERT(BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)));
	SJ_ASSERT(BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));
	
	Close(dummySocket);
}

void SocketManager::Cleanup()
{
	::WSACleanup();
}

bool SocketManager::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), &bytes, NULL, NULL);
}

SOCKET SocketManager::CreateSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

bool SocketManager::SetLinger(SOCKET socket, uint16_t onoff, uint16_t linger)
{
	LINGER option;
	option.l_onoff = onoff;
	option.l_linger = linger;
	return SetSockOpt(socket, SOL_SOCKET, SO_LINGER, option);
}

bool SocketManager::SetReuseAddress(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

bool SocketManager::SetRecvBufferSize(SOCKET socket, int32_t size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketManager::SetSendBufferSize(SOCKET socket, int32_t size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketManager::SetTcpNoDelay(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, TCP_NODELAY, flag);
}

// ListenSocket ¿É¼Ç -> ClientSocket
bool SocketManager::SetUpdateAccept(SOCKET socket, SOCKET listenSocket)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

bool SocketManager::Bind(SOCKET socket, SocketAddress socketAddress)
{
	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&socketAddress.GetSocketAddress()), sizeof(SOCKADDR_IN));
}

bool SocketManager::BindAnyAddress(SOCKET socket, uint16_t port)
{
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = ::htons(port);

	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress));
}

bool SocketManager::Listen(SOCKET socket, int32_t backlog)
{
	return SOCKET_ERROR != ::listen(socket, backlog);
}

void SocketManager::Close(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
		::closesocket(socket);
	socket = INVALID_SOCKET;
}
