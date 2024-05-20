#pragma once
#include "Session.h"

class Player;

class ClientSession : public ProtocolSession
{
public:
	~ClientSession();

	// Network
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvProtocol(BYTE* buffer, int32_t len) override;
	virtual void OnSend(int32_t len) override;

	// TODO
	// PreGame
	void ManageLogin(Protocol::UC_LOGIN& proto);
	void ManageCreatePlayer(Protocol::UC_CREATE_PLAYER& proto);
	void ManageEnterGame(Protocol::UC_ENTER_GAME& proto);

	// PingPong
	void Ping();
	void ManagePong();
	

public:
	std::atomic<std::shared_ptr<Player>> _player;

	std::vector<Protocol::LobbyPlayerInfo> _lobbyPlayers;
	Protocol::PlayerServerState _serverState = Protocol::SERVER_STATE_LOGIN;
	int32_t _accountDbId = 0;
	uint64_t _pingPongTick = 0;
	std::shared_ptr<Task> _task;

	// TEMP DUMMY TEST
	int32_t ID = 1;
};
