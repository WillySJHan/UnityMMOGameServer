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
bool Manage_UC_MOVE(std::shared_ptr<ProtocolSession>& session, Protocol::UC_MOVE& proto);
bool Manage_UC_SKILL(std::shared_ptr<ProtocolSession>& session, Protocol::UC_SKILL& proto);
bool Manage_UC_LOGIN(std::shared_ptr<ProtocolSession>& session, Protocol::UC_LOGIN& proto);
bool Manage_UC_CREATE_PLAYER(std::shared_ptr<ProtocolSession>& session, Protocol::UC_CREATE_PLAYER& proto);
bool Manage_UC_ENTER_GAME(std::shared_ptr<ProtocolSession>& session, Protocol::UC_ENTER_GAME& proto);
bool Manage_UC_EQUIP_ITEM(std::shared_ptr<ProtocolSession>& session, Protocol::UC_EQUIP_ITEM& proto);
bool Manage_UC_PONG(std::shared_ptr<ProtocolSession>& session, Protocol::UC_PONG& proto);
bool Manage_UC_CHANGE_ROOM(std::shared_ptr<ProtocolSession>& session, Protocol::UC_CHANGE_ROOM& proto);

class ClientProtocolManager
{
public:
	static void Init()
	{
		for (int32_t i = 0; i < UINT16_MAX; i++)
			g_protocolManager[i] = Manage_INVALID;
		g_protocolManager[UC_MOVE] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::UC_MOVE>(Manage_UC_MOVE, session, buffer, len); };
		g_protocolManager[UC_SKILL] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::UC_SKILL>(Manage_UC_SKILL, session, buffer, len); };
		g_protocolManager[UC_LOGIN] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::UC_LOGIN>(Manage_UC_LOGIN, session, buffer, len); };
		g_protocolManager[UC_CREATE_PLAYER] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::UC_CREATE_PLAYER>(Manage_UC_CREATE_PLAYER, session, buffer, len); };
		g_protocolManager[UC_ENTER_GAME] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::UC_ENTER_GAME>(Manage_UC_ENTER_GAME, session, buffer, len); };
		g_protocolManager[UC_EQUIP_ITEM] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::UC_EQUIP_ITEM>(Manage_UC_EQUIP_ITEM, session, buffer, len); };
		g_protocolManager[UC_PONG] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::UC_PONG>(Manage_UC_PONG, session, buffer, len); };
		g_protocolManager[UC_CHANGE_ROOM] = [](std::shared_ptr<ProtocolSession>& session, BYTE* buffer, uint32_t len) { return ManageProtocol<Protocol::UC_CHANGE_ROOM>(Manage_UC_CHANGE_ROOM, session, buffer, len); };
	}

	static bool ManageProtocol(std::shared_ptr<ProtocolSession>& session, BYTE* buffer, int32_t len)
	{
		ProtocolHeader* header = reinterpret_cast<ProtocolHeader*>(buffer);
		return g_protocolManager[header->id](session, buffer, len);
	}
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_ENTER_GAME& proto) { return MakeSendBuffer(proto, MS_ENTER_GAME); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_LEAVE_GAME& proto) { return MakeSendBuffer(proto, MS_LEAVE_GAME); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_SPAWN& proto) { return MakeSendBuffer(proto, MS_SPAWN); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_DESPAWN& proto) { return MakeSendBuffer(proto, MS_DESPAWN); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_MOVE& proto) { return MakeSendBuffer(proto, MS_MOVE); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_SKILL& proto) { return MakeSendBuffer(proto, MS_SKILL); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_DIE& proto) { return MakeSendBuffer(proto, MS_DIE); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_CHANGE_HP& proto) { return MakeSendBuffer(proto, MS_CHANGE_HP); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_CONNECTED& proto) { return MakeSendBuffer(proto, MS_CONNECTED); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_LOGIN& proto) { return MakeSendBuffer(proto, MS_LOGIN); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_CREATE_PLAYER& proto) { return MakeSendBuffer(proto, MS_CREATE_PLAYER); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_ITEM_LIST& proto) { return MakeSendBuffer(proto, MS_ITEM_LIST); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_ADD_ITEM& proto) { return MakeSendBuffer(proto, MS_ADD_ITEM); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_EQUIP_ITEM& proto) { return MakeSendBuffer(proto, MS_EQUIP_ITEM); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_CHANGE_STAT& proto) { return MakeSendBuffer(proto, MS_CHANGE_STAT); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::MS_PING& proto) { return MakeSendBuffer(proto, MS_PING); }

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
