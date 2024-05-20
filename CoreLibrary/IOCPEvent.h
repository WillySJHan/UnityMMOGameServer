#pragma once

class Session;

enum class EventType : uint8_t
{
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send
};

class IOCPEvent : public OVERLAPPED
{
public:
	IOCPEvent(EventType eventType);

	void			Init();

public:
	EventType eventType;
	std::shared_ptr<class IOCPObject> owner;
};

class ConnectEvent : public IOCPEvent
{
public:
	ConnectEvent() : IOCPEvent(EventType::Connect) { }
};

class DisconnectEvent : public IOCPEvent
{
public:
	DisconnectEvent() : IOCPEvent(EventType::Disconnect) { }
};

class AcceptEvent : public IOCPEvent
{
public:
	AcceptEvent() : IOCPEvent(EventType::Accept) { }

public:
	std::shared_ptr<class Session> session = nullptr;
};

class RecvEvent : public IOCPEvent
{
public:
	RecvEvent() : IOCPEvent(EventType::Recv) { }
};

class SendEvent : public IOCPEvent
{
public:
	SendEvent() : IOCPEvent(EventType::Send) { }

	std::vector<std::shared_ptr<class SendBuffer>> sendBuffers;
};

