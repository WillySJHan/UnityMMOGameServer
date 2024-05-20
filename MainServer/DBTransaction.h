#pragma once
#include "Data.h"

class Item;

class ItemDB
{
public:
	ItemDB();
	ItemDB(int32_t itemDbId, int32_t templateId, int32_t count, int32_t slot, bool equipped, int32_t ownerDbId);
public:
	int32_t ItemDbId = 0;
	int32_t TemplateId = 0;
	int32_t Count = 0;
	int32_t Slot = 0;
	bool Equipped = false;
	int32_t OwnerDbId = 0;
};



class DBTransaction : public TaskQueue
{
public:
	void SavePlayerStatus_Set(std::shared_ptr<Player> player, std::shared_ptr<Room> room);
	void SavePlayerStatus_Proc(std::shared_ptr<Player> player, std::shared_ptr<Room> room, Protocol::StatInfo stat, int32_t playerDbId);
	void RewardPlayer_Set(std::shared_ptr<Player> player, RewardData rewardData, std::shared_ptr<Room> room);
	void RewardPlayer_Proc(std::shared_ptr<Player> player, RewardData rewardData, std::shared_ptr<Room> room, int32_t slot);
	void EquipItemNoti_Set(std::shared_ptr<Player> player, std::shared_ptr<Item> item);
	void EquipItemNoti_Proc(std::shared_ptr<Player> player, int32_t itemDbId, bool equipped);
};

extern std::shared_ptr<DBTransaction> g_dbTransaction;
