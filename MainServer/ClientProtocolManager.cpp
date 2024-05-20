#include "pch.h"
#include "ClientProtocolManager.h"
#include "ClientSession.h"
#include "Player.h"
#include "Room.h"

FProtocolManager g_protocolManager[UINT16_MAX];

bool Manage_INVALID(std::shared_ptr<ProtocolSession>& session, BYTE* buffer, int32_t len)
{
	ProtocolHeader* header = reinterpret_cast<ProtocolHeader*>(buffer);
	// TODO : Log
	return false;
}

bool Manage_UC_MOVE(std::shared_ptr<ProtocolSession>& session, Protocol::UC_MOVE& proto)
{
	auto clientSession = std::static_pointer_cast<ClientSession>(session);

	auto player = clientSession->_player.load();

	if (player == nullptr)
		return false;

	std::shared_ptr<Room> room = player->_room.load().lock();
	if (room == nullptr)
		return false;

	room->PushTask(&Room::ManageMove, player, proto);

	return true;
}

bool Manage_UC_SKILL(std::shared_ptr<ProtocolSession>& session, Protocol::UC_SKILL& proto)
{
	auto clientSession = std::static_pointer_cast<ClientSession>(session);

	auto player = clientSession->_player.load();

	if (player == nullptr)
		return false;

	std::shared_ptr<Room> room = player->_room.load().lock();
	if (room == nullptr)
		return false;

	room->PushTask(&Room::ManageSkill, player, proto);

	return true;
}

bool Manage_UC_LOGIN(std::shared_ptr<ProtocolSession>& session, Protocol::UC_LOGIN& proto)
{
	auto clientSession = std::static_pointer_cast<ClientSession>(session);

	clientSession->ID = proto.room_id();

	clientSession->ManageLogin(proto);

	return true;
}

bool Manage_UC_CREATE_PLAYER(std::shared_ptr<ProtocolSession>& session, Protocol::UC_CREATE_PLAYER& proto)
{
	auto clientSession = std::static_pointer_cast<ClientSession>(session);
	clientSession->ManageCreatePlayer(proto);

	return true;
}

bool Manage_UC_ENTER_GAME(std::shared_ptr<ProtocolSession>& session, Protocol::UC_ENTER_GAME& proto)
{
	auto clientSession = std::static_pointer_cast<ClientSession>(session);
	clientSession->ManageEnterGame(proto);

	return true;
}

bool Manage_UC_EQUIP_ITEM(std::shared_ptr<ProtocolSession>& session, Protocol::UC_EQUIP_ITEM& proto)
{
	auto clientSession = std::static_pointer_cast<ClientSession>(session);

	auto player = clientSession->_player.load();

	if (player == nullptr)
		return false;

	std::shared_ptr<Room> room = player->_room.load().lock();
	if (room == nullptr)
		return false;

	room->PushTask(&Room::ManageEquipItem, player, proto);

	return true;
}

bool Manage_UC_PONG(std::shared_ptr<ProtocolSession>& session, Protocol::UC_PONG& proto)
{
	std::shared_ptr<ClientSession> clientSession = std::static_pointer_cast<ClientSession>(session);

	clientSession->ManagePong();


	return true;
}

bool Manage_UC_CHANGE_ROOM(std::shared_ptr<ProtocolSession>& session, Protocol::UC_CHANGE_ROOM& proto)
{
	std::shared_ptr<ClientSession> clientSession = std::static_pointer_cast<ClientSession>(session);

	auto player = clientSession->_player.load();

	if (player == nullptr)
		return false;

	auto room = player->_room.load().lock();

	if (room == nullptr)
		return false;

	room->PushTask(&Room::ChangeRoom, player, proto);

	return true;
}




