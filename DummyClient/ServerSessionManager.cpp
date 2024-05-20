#include "pch.h"
#include "ServerSessionManager.h"
#include "ServerSession.h"

ServerSessionManager g_sessionManager;

void ServerSessionManager::Add(std::shared_ptr<ServerSession> session)
{
	std::lock_guard<std::recursive_mutex> lock(_rMutex);
	_sessions.insert(session);
	session->_dummyId = _counter;
	_counter++;

	std::cout << "Connected " << _sessions.size() << " Players" << std::endl;
}

void ServerSessionManager::Remove(std::shared_ptr<ServerSession> session)
{
	std::lock_guard<std::recursive_mutex> lock(_rMutex);
	_sessions.erase(session);

	std::cout << "disonnected " << _sessions.size() << " Players" << std::endl;
}

