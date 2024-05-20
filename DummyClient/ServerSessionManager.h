#pragma once

class ServerSession;

class ServerSessionManager
{
public:
	void Add(std::shared_ptr<ServerSession> session);
	void Remove(std::shared_ptr<ServerSession> session);

private:
	std::recursive_mutex _rMutex;
	std::unordered_set<std::shared_ptr<ServerSession>> _sessions;
	int32_t _counter = 1;
};

extern ServerSessionManager g_sessionManager;
