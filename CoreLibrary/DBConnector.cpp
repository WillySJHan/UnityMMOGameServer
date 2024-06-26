#include "pch.h"
#include "DBConnector.h"

bool DBConnector::Connect(SQLHENV henv, const wchar_t* connectionString)
{
	if (::SQLAllocHandle(SQL_HANDLE_DBC, henv, &_connector) != SQL_SUCCESS)
		return false;

	WCHAR stringBuffer[MAX_PATH] = { 0 };
	::wcscpy_s(stringBuffer, connectionString);

	WCHAR resultString[MAX_PATH] = { 0 };
	SQLSMALLINT resultStringLen = 0;

	SQLRETURN ret = ::SQLDriverConnectW(
		_connector,
		NULL,
		reinterpret_cast<SQLWCHAR*>(stringBuffer),
		_countof(stringBuffer),
		OUT reinterpret_cast<SQLWCHAR*>(resultString),
		_countof(resultString),
		OUT &resultStringLen,
		SQL_DRIVER_NOPROMPT
	);

	if (::SQLAllocHandle(SQL_HANDLE_STMT, _connector, &_statement) != SQL_SUCCESS)
		return false;

	return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

void DBConnector::Clear()
{
	if (_connector != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_DBC, _connector);
		_connector = SQL_NULL_HANDLE;
	}

	if (_statement != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_STMT, _statement);
		_statement = SQL_NULL_HANDLE;
	}
}

bool DBConnector::Execute(const wchar_t* query)
{
	SQLRETURN ret = ::SQLExecDirectW(_statement, (SQLWCHAR*)query, SQL_NTSL);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return true;

	ManageError(ret);
	return false;
}

bool DBConnector::Fetch()
{
	SQLRETURN ret = ::SQLFetch(_statement);

	switch (ret)
	{
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
		return true;
	case SQL_NO_DATA:
		return false;
	case SQL_ERROR:
		ManageError(ret);
		return false;
	default:
		return true;
	}
}

int32_t DBConnector::GetRowCount()
{
	SQLLEN count = 0;
	SQLRETURN ret = ::SQLRowCount(_statement, &count);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return static_cast<int32_t>(count);

	return -1;
}

void DBConnector::Unbind()
{
	::SQLFreeStmt(_statement, SQL_UNBIND);
	::SQLFreeStmt(_statement, SQL_RESET_PARAMS);
	::SQLFreeStmt(_statement, SQL_CLOSE);
}

bool DBConnector::BindParam(int32_t paramIndex, bool* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, static_cast<int32_t>(sizeof(bool)), value, index);
}

bool DBConnector::BindParam(int32_t paramIndex, float* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_FLOAT, SQL_REAL, 0, value, index);
}
__int8 a = 1;
bool DBConnector::BindParam(int32_t paramIndex, double* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_DOUBLE, SQL_DOUBLE, 0, value, index);
}

bool DBConnector::BindParam(int32_t paramIndex, int8_t* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, static_cast<int32_t>(sizeof(int8_t)), value, index);
}

bool DBConnector::BindParam(int32_t paramIndex, int16_t* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_SHORT, SQL_SMALLINT, static_cast<int32_t>(sizeof(int16_t)), value, index);
}

bool DBConnector::BindParam(int32_t paramIndex, int32_t* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_LONG, SQL_INTEGER, static_cast<int32_t>(sizeof(int32_t)), value, index);
}

bool DBConnector::BindParam(int32_t paramIndex, int64_t* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_SBIGINT, SQL_BIGINT, static_cast<int32_t>(sizeof(int64_t)), value, index);
}

bool DBConnector::BindParam(int32_t paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, static_cast<int32_t>(sizeof(TIMESTAMP_STRUCT)), value, index);
}

bool DBConnector::BindParam(int32_t paramIndex, const wchar_t* str, SQLLEN* index)
{
	SQLULEN size = static_cast<SQLULEN>((::wcslen(str) + 1) * 2);
	*index = SQL_NTSL;

	if (size > WVARCHAR_MAX)
		return BindParam(paramIndex, SQL_C_WCHAR, SQL_WLONGVARCHAR, size, (SQLPOINTER)str, index);
	else
		return BindParam(paramIndex, SQL_C_WCHAR, SQL_WVARCHAR, size, (SQLPOINTER)str, index);
}

bool DBConnector::BindParam(int32_t paramIndex, const BYTE* bin, int32_t size, SQLLEN* index)
{
	if (bin == nullptr)
	{
		*index = SQL_NULL_DATA;
		size = 1;
	}
	else
		*index = size;

	if (size > BINARY_MAX)
		return BindParam(paramIndex, SQL_C_BINARY, SQL_LONGVARBINARY, size, (BYTE*)bin, index);
	else
		return BindParam(paramIndex, SQL_C_BINARY, SQL_BINARY, size, (BYTE*)bin, index);
}

bool DBConnector::BindCol(int32_t columnIndex, bool* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_TINYINT, static_cast<int32_t>(sizeof(bool)), value, index);
}

bool DBConnector::BindCol(int32_t columnIndex, float* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_FLOAT, static_cast<int32_t>(sizeof(float)), value, index);
}

bool DBConnector::BindCol(int32_t columnIndex, double* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_DOUBLE, static_cast<int32_t>(sizeof(double)), value, index);
}

bool DBConnector::BindCol(int32_t columnIndex, int8_t* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_TINYINT, static_cast<int32_t>(sizeof(__int8)), value, index);
}

bool DBConnector::BindCol(int32_t columnIndex, int16_t* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_SHORT, static_cast<int32_t>(sizeof(__int16)), value, index);
}

bool DBConnector::BindCol(int32_t columnIndex, int32_t* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_LONG, static_cast<int32_t>(sizeof(__int32)), value, index);
}

bool DBConnector::BindCol(int32_t columnIndex, int64_t* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_SBIGINT, static_cast<int32_t>(sizeof(__int64)), value, index);
}

bool DBConnector::BindCol(int32_t columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_TYPE_TIMESTAMP, static_cast<int32_t>(sizeof(TIMESTAMP_STRUCT)), value, index);
}

bool DBConnector::BindCol(int32_t columnIndex, WCHAR* str, int32_t size, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_WCHAR, size, str, index);
}

bool DBConnector::BindCol(int32_t columnIndex, BYTE* bin, int32_t size, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_BINARY, size, bin, index);
}

bool DBConnector::BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index)
{
	SQLRETURN ret = ::SQLBindParameter(_statement, paramIndex, SQL_PARAM_INPUT, cType, sqlType, len, 0, ptr, 0, index);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		ManageError(ret);
		return false;
	}

	return true;
}

bool DBConnector::BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index)
{
	SQLRETURN ret = ::SQLBindCol(_statement, columnIndex, cType, value, len, index);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		ManageError(ret);
		return false;
	}

	return true;
}

void DBConnector::ManageError(SQLRETURN ret)
{
	if (ret == SQL_SUCCESS)
		return;

	SQLSMALLINT index = 1;
	SQLWCHAR sqlState[MAX_PATH] = { 0 };
	SQLINTEGER nativeErr = 0;
	SQLWCHAR errMsg[MAX_PATH] = { 0 };
	SQLSMALLINT msgLen = 0;
	SQLRETURN errorRet = 0;

	while (true)
	{
		errorRet = ::SQLGetDiagRecW(
			SQL_HANDLE_STMT,
			_statement,
			index,
			sqlState,
			OUT &nativeErr,
			errMsg,
			_countof(errMsg),
			OUT &msgLen
		);

		if (errorRet == SQL_NO_DATA)
			break;

		if (errorRet != SQL_SUCCESS && errorRet != SQL_SUCCESS_WITH_INFO)
			break;

		// TODO : Log
		std::wcout.imbue(std::locale("kor"));
		std::wcout << errMsg << std::endl;

		index++;
	}
}