#include "pch.h"
#include "ServerProtocolManager.h"
#include "Protocol.pb.h"

FProtocolManager g_protocolManager[UINT16_MAX];

bool Manage_INVALID(std::shared_ptr<ProtocolSession>& session, BYTE* buffer, int32_t len)
{
	ProtocolHeader* header = reinterpret_cast<ProtocolHeader*>(buffer);
	// TODO : Log
	return false;
}

bool Manage_MS_ENTER_GAME(std::shared_ptr<ProtocolSession>& session, Protocol::MS_ENTER_GAME& proto)
{

	return true;
}

bool Manage_MS_LEAVE_GAME(std::shared_ptr<ProtocolSession>& session, Protocol::MS_LEAVE_GAME& proto)
{

	return true;
}

bool Manage_MS_SPAWN(std::shared_ptr<ProtocolSession>& session, Protocol::MS_SPAWN& proto)
{

	return true;
}

bool Manage_MS_DESPAWN(std::shared_ptr<ProtocolSession>& session, Protocol::MS_DESPAWN& proto)
{

	return true;
}

bool Manage_MS_MOVE(std::shared_ptr<ProtocolSession>& session, Protocol::MS_MOVE& proto)
{

	return true;
}

bool Manage_MS_SKILL(std::shared_ptr<ProtocolSession>& session, Protocol::MS_SKILL& proto)
{

	return true;
}

bool Manage_MS_DIE(std::shared_ptr<ProtocolSession>& session, Protocol::MS_DIE& proto)
{

	return true;
}

bool Manage_MS_CHANGE_HP(std::shared_ptr<ProtocolSession>& session, Protocol::MS_CHANGE_HP& proto)
{

	return true;
}

bool Manage_MS_CONNECTED(std::shared_ptr<ProtocolSession>& session, Protocol::MS_CONNECTED& proto)
{
	std::shared_ptr<ServerSession> serverSession = std::static_pointer_cast<ServerSession>(session);
	std::wstring dummyClientName = L"DummyClient_" + std::to_wstring(serverSession->_dummyId);

	Protocol::UC_LOGIN loginProto;

	loginProto.set_unique_id(Utility::WstrTostr(dummyClientName));
	loginProto.set_room_id(2);

	auto sendBuffer = ServerProtocolManager::MakeSendBuffer(loginProto);
	serverSession->Send(sendBuffer);

	return true;
}

bool Manage_MS_LOGIN(std::shared_ptr<ProtocolSession>& session, Protocol::MS_LOGIN& proto)
{
	std::shared_ptr<ServerSession> serverSession = std::static_pointer_cast<ServerSession>(session);

	// TODO : 로비 UI에서 캐릭터 보여주고, 선택할 수 있도록
	if (proto.players_size() == 0)
	{
		Protocol::UC_CREATE_PLAYER createProto;
		std::wstring dummyClientName = L"DummyClient_" + std::to_wstring(serverSession->_dummyId);
		createProto.set_name(Utility::WstrTostr(dummyClientName));

		auto sendBuffer = ServerProtocolManager::MakeSendBuffer(createProto);
		serverSession->Send(sendBuffer);
	}
	else
	{
		// 무조건 첫번째 로그인
		Protocol::LobbyPlayerInfo info = proto.players(0);
		Protocol::UC_ENTER_GAME enterGameProto;
		enterGameProto.set_name(info.name());

		auto sendBuffer = ServerProtocolManager::MakeSendBuffer(enterGameProto);
		serverSession->Send(sendBuffer);
	}

	return true;
}

bool Manage_MS_CREATE_PLAYER(std::shared_ptr<ProtocolSession>& session, Protocol::MS_CREATE_PLAYER& proto)
{
	std::shared_ptr<ServerSession> serverSession = std::static_pointer_cast<ServerSession>(session);
	if (!proto.has_player())
	{
		// 생략 이름겹침
	}
	else
	{
		Protocol::UC_ENTER_GAME enterGameProto;
		enterGameProto.set_name(proto.mutable_player()->name());

		auto sendBuffer = ServerProtocolManager::MakeSendBuffer(enterGameProto);
		serverSession->Send(sendBuffer);
	}


	return true;
}

bool Manage_MS_ITEM_LIST(std::shared_ptr<ProtocolSession>& session, Protocol::MS_ITEM_LIST& proto)
{

	return true;
}

bool Manage_MS_ADD_ITEM(std::shared_ptr<ProtocolSession>& session, Protocol::MS_ADD_ITEM& proto)
{

	return true;
}

bool Manage_MS_EQUIP_ITEM(std::shared_ptr<ProtocolSession>& session, Protocol::MS_EQUIP_ITEM& proto)
{

	return true;
}

bool Manage_MS_CHANGE_STAT(std::shared_ptr<ProtocolSession>& session, Protocol::MS_CHANGE_STAT& proto)
{

	return true;
}

bool Manage_MS_PING(std::shared_ptr<ProtocolSession>& session, Protocol::MS_PING& proto)
{

	return true;
}






