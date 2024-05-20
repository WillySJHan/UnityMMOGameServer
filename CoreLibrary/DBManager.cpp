#include "pch.h"
#include "DBManager.h"

DBManager::DBManager()
{
}

DBManager::~DBManager()
{
	Clear();
}

bool DBManager::Connect(int32_t connectorCount, const WCHAR* connectorString)
{
	std::lock_guard<std::recursive_mutex> lock(_rMutex);
	if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_env) != SQL_SUCCESS)
		return false;

	if (::SQLSetEnvAttr(_env, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS)
		return false;

	for (int32_t i = 0; i < connectorCount; i++)
	{
		DBConnector* connector = new DBConnector();
		if (connector->Connect(_env, connectorString) == false)
			return false;

		_connectors.push_back(connector);
	}

	return true;
}

void DBManager::Clear()
{
	std::lock_guard<std::recursive_mutex> lock(_rMutex);
	if (_env != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_ENV, _env);
		_env = SQL_NULL_HANDLE;
	}

	for (DBConnector* connection : _connectors)
		delete(connection);

	_connectors.clear();
}

DBConnector* DBManager::Pop()
{
	std::lock_guard<std::recursive_mutex> lock(_rMutex);
	if (_connectors.empty())
		return nullptr;

	DBConnector* connection = _connectors.back();
	_connectors.pop_back();
	return connection;
}

void DBManager::Push(DBConnector* connector)
{
	std::lock_guard<std::recursive_mutex> lock(_rMutex);

	_connectors.push_back(connector);
}