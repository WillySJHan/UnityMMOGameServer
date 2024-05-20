#include "pch.h"
#include "ServerSession.h"

#include "ServerProtocolManager.h"
#include "ServerSessionManager.h"

ServerSession::~ServerSession()
{
	std::cout << "~ServerSession" << std::endl;
}

void ServerSession::OnConnected()
{
	std::cout << "onConnected" << std::endl;
	g_sessionManager.Add(std::static_pointer_cast<ServerSession>(shared_from_this()));
}

void ServerSession::OnRecvProtocol(BYTE* buffer, int32_t len)
{
	std::shared_ptr<ProtocolSession> session = GetProtocolSessionSptr();
	ProtocolHeader* header = reinterpret_cast<ProtocolHeader*>(buffer);

	ServerProtocolManager::ManageProtocol(session, buffer, len);
}

void ServerSession::OnSend(int len)
{
	
}

void ServerSession::OnDisconnected()
{
	std::cout << "Disconnected" << std::endl;
	g_sessionManager.Remove(std::static_pointer_cast<ServerSession>(shared_from_this()));
}