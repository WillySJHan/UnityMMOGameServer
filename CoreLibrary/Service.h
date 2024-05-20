#pragma once
#include "SocketAddress.h"
#include "IOCPBase.h"
#include "Listener.h"
#include <functional>

enum class ServiceType : uint8_t
{
	Server,
	Client
};

using FSessionMaker = std::function<std::shared_ptr<class Session>(void)>;

class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(ServiceType type, SocketAddress address, std::shared_ptr<IOCPBase> base, FSessionMaker f_maker, int32_t maxSessionCount = 1);
	virtual ~Service();

	virtual bool		Start() = 0;
	bool				CanStart() { return _f_sessionMaker != nullptr; }

	virtual void		CloseService();
	void				SetSessionFactory(FSessionMaker func) { _f_sessionMaker = func; }


	std::shared_ptr<class Session>		CreateSession();
	void								AddSession(std::shared_ptr<class Session> session);
	void								ReleaseSession(std::shared_ptr<class Session> session);
	int32_t								GetCurrentSessionCount() { return _sessionCount; }
	int32_t								GetMaxSessionCount() { return _maxSessionCount; }

public:
	ServiceType						GetServiceType() { return _type; }
	SocketAddress					GetSocketAddress() { return _socketAddress; }
	//TODO
	std::shared_ptr<IOCPBase>&		GetIocpBase() { return _iocpBase; }

protected:
	std::recursive_mutex			_rMutex;
	ServiceType					_type;
	SocketAddress				_socketAddress = {};
	std::shared_ptr<IOCPBase>	_iocpBase;

	std::set<std::shared_ptr<class Session>>	_sessions;
	int32_t										_sessionCount = 0;
	int32_t										_maxSessionCount = 0;
	FSessionMaker								_f_sessionMaker;
};


class ClientService : public Service
{
public:
	ClientService(SocketAddress targetAddress, std::shared_ptr<IOCPBase> base, FSessionMaker f_maker, int32_t maxSessionCount = 1);
	virtual ~ClientService() {}

	virtual bool	Start() override;
};


class ServerService : public Service
{
public:
	ServerService(SocketAddress serverAddress, std::shared_ptr<IOCPBase> base, FSessionMaker f_maker, int32_t maxSessionCount = 1);
	virtual ~ServerService() {}

	virtual bool	Start() override;
	virtual void	CloseService() override;

private:
	std::shared_ptr<Listener>		_listener = nullptr;
};

