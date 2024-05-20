#pragma once

#include "DBConnector.h"

template<int32_t C>
struct FullBits { enum { value = (1 << (C - 1)) | FullBits<C - 1>::value }; };

template<>
struct FullBits<1> { enum { value = 1 }; };

template<>
struct FullBits<0> { enum { value = 0 }; };

template<int32_t ParamCount, int32_t ColumnCount>
class DBBind
{
public:
	DBBind(DBConnector& dbConnector, const wchar_t* query)
		: _dbConnector(dbConnector), _query(query)
	{
		::memset(_paramIndex, 0, sizeof(_paramIndex));
		::memset(_columnIndex, 0, sizeof(_columnIndex));
		_paramFlag = 0;
		_columnFlag = 0;
		dbConnector.Unbind();
	}

	bool Validate()
	{
		return _paramFlag == FullBits<ParamCount>::value && _columnFlag == FullBits<ColumnCount>::value;
	}

	bool Execute() 
	{
		SJ_ASSERT(Validate());

		return _dbConnector.Execute(_query);
	}

	bool Fetch()
	{
		return _dbConnector.Fetch();
	}

public:
	template<typename T>
	void BindParam(int32_t idx, T& value)
	{
		_dbConnector.BindParam(idx + 1, &value, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	void BindParam(int32_t idx, const WCHAR* value)
	{
		_dbConnector.BindParam(idx + 1, value, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	template<typename T, int32_t N>
	void BindParam(int32_t idx, T(&value)[N])
	{
		_dbConnector.BindParam(idx + 1, (const BYTE*)value, static_cast<int32_t>(sizeof(T)) * N, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	template<typename T>
	void BindParam(int32_t idx, T* value, int32_t N)
	{
		_dbConnector.BindParam(idx + 1, (const BYTE*)value, static_cast<int32_t>(sizeof(T)) * N, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	template<typename T>
	void BindCol(int32_t idx, T& value)
	{
		_dbConnector.BindCol(idx + 1, &value, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	template<int32_t N>
	void BindCol(int32_t idx, wchar_t(&value)[N])
	{
		_dbConnector.BindCol(idx + 1, value, N - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	void BindCol(int32_t idx, wchar_t* value, int32_t len)
	{
		_dbConnector.BindCol(idx + 1, value, len - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	template<typename T, int32_t N>
	void BindCol(int32_t idx, T(&value)[N])
	{
		_dbConnector.BindCol(idx + 1, value, static_cast<int32_t>(sizeof(T)) * N, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

protected:
	DBConnector& _dbConnector;
	const wchar_t*	_query;
	SQLLEN			_paramIndex[ParamCount > 0 ? ParamCount : 1];
	SQLLEN			_columnIndex[ColumnCount > 0 ? ColumnCount : 1];
	uint64_t		_paramFlag;
	uint64_t		_columnFlag;
};
