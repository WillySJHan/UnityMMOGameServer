#pragma once
#include "IOCPBase.h"
#include "IOCPEvent.h"
#include "SocketAddress.h"
#include "RecvBuffer.h"

class Session : public IOCPObject
{
	friend class IocpBase;
	friend class Service;
	friend class Listener;

	enum { BUFFER_SIZE = 0x10000, }; // 64KB


public:
	Session();
	virtual ~Session();

public:

	/* 외부에서 사용 */
	void				Send(std::shared_ptr<class SendBuffer> sendBuffer);
	bool				Connect();
	void				Disconnect(const wchar_t* cause);

	std::shared_ptr<class Service>	GetService() { return _service.lock(); }
	void							SetService(std::shared_ptr<class Service> service) { _service = service; }

public:
	/* 정보 관련 */
	void						SetSocketAddress(SocketAddress address) { _socketAddress = address; }
	SocketAddress				GetAddress() { return _socketAddress; }
	SOCKET						GetSocket() { return _socket; }
	bool						IsConnected() { return _connected; }
	std::shared_ptr<Session>	GetSessionPtr() { return std::static_pointer_cast<Session>(shared_from_this()); }

private:
	/* 인터페이스 구현 */
	virtual HANDLE				GetHandle() override;
	virtual void				Dispatch(class IOCPEvent* iocpEvent, int32_t bytes = 0) override;

private:
	bool				RegisterConnect();
	bool				RegisterDisconnect();
	void				RegisterRecv();
	void				RegisterSend();

	void				ProcessConnect();
	void				ProcessDisconnect();
	void				ProcessRecv(int32_t bytes);
	void				ProcessSend(int32_t bytes);

	void				HandleError(int32_t errorCode);

protected:
	/* 컨텐츠 코드에서 오버라이딩 */
	virtual void		OnConnected() { }
	virtual int32_t		OnRecv(BYTE* buffer, int32_t len) { return len; }
	virtual void		OnSend(int32_t len) { }
	virtual void		OnDisconnected() { }


private:
	std::weak_ptr<class Service>	_service;
	SOCKET							_socket = INVALID_SOCKET;
	SocketAddress					_socketAddress = {};
	std::atomic<bool>				_connected = false;



private:
	//std::shared_mutex _sMutex;
	std::recursive_mutex _rMutex;

	RecvBuffer _recvBuffer;

	std::queue<std::shared_ptr<SendBuffer>> _sendQueue;
	std::atomic<bool> _sendRegistered = false;

private:
	/* IocpEvent 재사용 */
	ConnectEvent		_connectEvent;
	DisconnectEvent		_disconnectEvent;
	RecvEvent			_recvEvent;
	SendEvent			_sendEvent;
};

struct ProtocolHeader
{
	uint16_t size;
	uint16_t id; 
};

class ProtocolSession : public Session
{
public:
	ProtocolSession();
	virtual ~ProtocolSession();

	std::shared_ptr<ProtocolSession>	GetProtocolSessionSptr() { return std::static_pointer_cast<ProtocolSession>(shared_from_this()); }

protected:
	virtual int32_t		OnRecv(BYTE* buffer, int32_t len) final;
	virtual void		OnRecvProtocol(BYTE* buffer, int32_t len) = 0;
};