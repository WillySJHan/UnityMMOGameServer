#pragma once


class ServerSession : public ProtocolSession
{
public:
	~ServerSession();

	virtual void OnConnected() override;

	virtual void OnRecvProtocol(BYTE* buffer, int32_t len) override;

	virtual void OnSend(int32_t len) override;

	virtual void OnDisconnected() override;

public:
	int32_t _dummyId = 0;
};
