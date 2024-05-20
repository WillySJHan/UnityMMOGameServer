#pragma once
#include "Protocol.pb.h"

using FProtocolManager = std::function<bool(std::shared_ptr<ProtocolSession>&, BYTE*, int32_t)>;
extern FProtocolManager g_protocolManager[UINT16_MAX];

enum : uint16_t
{
	MS_ENTER_GAME = 1000,
	MS_LEAVE_GAME = 1001,
	MS_SPAWN = 1002,
	MS_DESPAWN = 1003,
	UC_MOVE = 1004,
	MS_MOVE = 1005,
	UC_SKILL = 1006,
	MS_SKILL = 1007,
	MS_DIE = 1008,
	MS_CHANGE_HP = 1009,
	MS_CONNECTED = 1010,
	UC_LOGIN = 1011,
	MS_LOGIN = 1012,
	UC_CREATE_PLAYER = 1013,
	MS_CREATE_PLAYER = 1014,
	UC_ENTER_GAME = 1015,
	MS_ITEM_LIST = 1016,
	MS_ADD_ITEM = 1017,
	UC_EQUIP_ITEM = 1018,
	MS_EQUIP_ITEM = 1019,
	MS_CHANGE_STAT = 1020,
	MS_PING = 1021,
	UC_PONG = 1022,
	UC_CHANGE_ROOM = 1023,
};

bool Manage_INVALID(std::shared_ptr<ProtocolSession>& session, BYTE* buffer, int32_t len);
bool Manage_MS_ENTER_GAME(std::shared_ptr<ProtocolSession>& session, Protocol::MS_ENTER_GAME& proto);
bool Manage_MS_LEAVE_GAME(std::shared_ptr<ProtocolSession>& session, Protocol::MS_LEAVE_GAME& proto);
bool Manage_MS_SPAWN(std::shared_ptr<ProtocolSession>& session, Protocol::MS_SPAWN& proto);
bool Manage_MS_DESPAWN(std::shared_ptr<ProtocolSession>& session, Protocol::MS_DESPAWN& proto);
bool Manage_MS_MOVE(std::shared_ptr<ProtocolSession>& session, Protocol::MS_MOVE& proto);
bool Manage_MS_SKILL(std::shared_ptr<ProtocolSession>& session, Protocol::MS_SKILL& proto);
bool Manage_MS_DIE(std::shared_ptr<ProtocolSession>& session, Protocol::MS_DIE& proto);
bool Manage_MS_CHANGE_HP(std::shared_ptr<ProtocolSession>& session, Protocol::MS_CHANGE_HP& proto);
bool Manage_MS_CONNECTED(std::shared_ptr<ProtocolSession>& session, Protocol::MS_CONNECTED& proto);
bool Manage_MS_LOGIN(std::shared_ptr<ProtocolSession>& session, Protocol::MS_LOGIN& proto);
bool Manage_MS_CREATE_PLAYER(std::shared_ptr<ProtocolSession>& session, Protocol::MS_CREATE_PLAYER& proto);
bool Manage_MS_ITEM_LIST(std::shared_ptr<ProtocolSession>& session, Protocol::MS_ITEM_LIST& proto);
bool Manage_MS_ADD_ITEM(std::shared_ptr<ProtocolSession>& session, Protocol::MS_ADD_ITEM& proto);
bool Manage_MS_EQUIP_ITEM(std::shared_ptr<ProtocolSession>& session, Protocol::MS_EQUIP_ITEM& proto);
bool Manage_MS_CHANGE_STAT(std::shared_ptr<ProtocolSession>& session, Protocol::MS_CHANGE_STAT& proto);
bool Manage_MS_PING(std::shared_ptr<ProtocolSession>& session, Protocol::MS_PING& proto);

class ServerProtocolManager
{
public:
	static void Init()
	{
		for (int32_t i = 0; i < UINT16_MAX; i++)
			g_protocolManager[i] = Manage_INVALID;
		g_protocolManager[MS_ENTER_GAME] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_ENTER_GAME>(Manage_MS_ENTER_GAME, session, buffer, len); };
		g_protocolManager[MS_LEAVE_GAME] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_LEAVE_GAME>(Manage_MS_LEAVE_GAME, session, buffer, len); };
		g_protocolManager[MS_SPAWN] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_SPAWN>(Manage_MS_SPAWN, session, buffer, len); };
		g_protocolManager[MS_DESPAWN] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_DESPAWN>(Manage_MS_DESPAWN, session, buffer, len); };
		g_protocolManager[MS_MOVE] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_MOVE>(Manage_MS_MOVE, session, buffer, len); };
		g_protocolManager[MS_SKILL] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_SKILL>(Manage_MS_SKILL, session, buffer, len); };
		g_protocolManager[MS_DIE] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_DIE>(Manage_MS_DIE, session, buffer, len); };
		g_protocolManager[MS_CHANGE_HP] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_CHANGE_HP>(Manage_MS_CHANGE_HP, session, buffer, len); };
		g_protocolManager[MS_CONNECTED] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_CONNECTED>(Manage_MS_CONNECTED, session, buffer, len); };
		g_protocolManager[MS_LOGIN] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_LOGIN>(Manage_MS_LOGIN, session, buffer, len); };
		g_protocolManager[MS_CREATE_PLAYER] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_CREATE_PLAYER>(Manage_MS_CREATE_PLAYER, session, buffer, len); };
		g_protocolManager[MS_ITEM_LIST] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_ITEM_LIST>(Manage_MS_ITEM_LIST, session, buffer, len); };
		g_protocolManager[MS_ADD_ITEM] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_ADD_ITEM>(Manage_MS_ADD_ITEM, session, buffer, len); };
		g_protocolManager[MS_EQUIP_ITEM] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_EQUIP_ITEM>(Manage_MS_EQUIP_ITEM, session, buffer, len); };
		g_protocolManager[MS_CHANGE_STAT] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_CHANGE_STAT>(Manage_MS_CHANGE_STAT, session, buffer, len); };
		g_protocolManager[MS_PING] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::MS_PING>(Manage_MS_PING, session, buffer, len); };
	}

	static bool ManageProtocol(std::shared_ptr<ProtocolSession>& session, BYTE* buffer, int32_t len)
	{
		ProtocolHeader* header = reinterpret_cast<ProtocolHeader*>(buffer);
		return g_protocolManager[header->id](session, buffer, len);
	}
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::UC_MOVE& proto) { return MakeSendBuffer(proto, UC_MOVE); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::UC_SKILL& proto) { return MakeSendBuffer(proto, UC_SKILL); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::UC_LOGIN& proto) { return MakeSendBuffer(proto, UC_LOGIN); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::UC_CREATE_PLAYER& proto) { return MakeSendBuffer(proto, UC_CREATE_PLAYER); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::UC_ENTER_GAME& proto) { return MakeSendBuffer(proto, UC_ENTER_GAME); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::UC_EQUIP_ITEM& proto) { return MakeSendBuffer(proto, UC_EQUIP_ITEM); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::UC_PONG& proto) { return MakeSendBuffer(proto, UC_PONG); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::UC_CHANGE_ROOM& proto) { return MakeSendBuffer(proto, UC_CHANGE_ROOM); }

private:
	template<typename ProtocolType, typename FManage>
	static bool ManageProtocol(FManage f_manage, std::shared_ptr<ProtocolSession>& session, BYTE* buffer, int32_t len)
	{
		ProtocolType proto;
		if (proto.ParseFromArray(buffer + sizeof(ProtocolHeader), len - sizeof(ProtocolHeader)) == false)
			return false;

		return f_manage(session, proto);
	}

	template<typename T>
	static std::shared_ptr<SendBuffer> MakeSendBuffer(T& proto, uint16_t protoId)
	{
		const uint16_t dataSize = static_cast<uint16_t>(proto.ByteSizeLong());
		const uint16_t packetSize = dataSize + sizeof(ProtocolHeader);

		std::shared_ptr<SendBuffer> sendBuffer = std::make_shared<SendBuffer>(packetSize);
		ProtocolHeader* header = reinterpret_cast<ProtocolHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = protoId;
		SJ_ASSERT(proto.SerializeToArray(&header[1], dataSize));
		sendBuffer->SetWriteSize(packetSize);

		return sendBuffer;
	}
};
