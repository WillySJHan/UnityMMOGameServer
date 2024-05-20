#pragma once
#include "DBConnector.h"

class DBManager
{
public:
	DBManager();
	~DBManager();

	bool Connect(int32_t connectorCount, const wchar_t* connectorString);
	void Clear();

	DBConnector* Pop();
	void Push(DBConnector* connector);

private:
	std::recursive_mutex _rMutex;
	SQLHENV _env = SQL_NULL_HANDLE;
	// TODO queue»ç¿ë?
	std::vector<DBConnector*> _connectors;
};
