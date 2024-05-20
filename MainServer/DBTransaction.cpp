#include "pch.h"
#include "DBTransaction.h"

#include "DBBind.h"
#include "DBConnector.h"
#include "DBManager.h"
#include "Inventory.h"
#include "Item.h"
#include "Room.h"

std::shared_ptr<DBTransaction> g_dbTransaction = std::make_shared<DBTransaction>();

ItemDB::ItemDB()
{
}

ItemDB::ItemDB(int32_t itemDbId, int32_t templateId, int32_t count, int32_t slot, bool equipped, int32_t ownerDbId)
	: ItemDbId(itemDbId), TemplateId(templateId), Count(count), Slot(slot), Equipped(equipped), OwnerDbId(ownerDbId)
{
}

void DBTransaction::SavePlayerStatus_Set(std::shared_ptr<Player> player, std::shared_ptr<Room> room)
{
	if (player == nullptr || room == nullptr)
		return;

	PushTask(&DBTransaction::SavePlayerStatus_Proc,player, room, *player->_statInfo, player->_playerDbId);

	
}

void DBTransaction::SavePlayerStatus_Proc(std::shared_ptr<Player> player, std::shared_ptr<Room> room,
	Protocol::StatInfo stat, int32_t playerDbId)
{
	if (player == nullptr || room == nullptr)
		return;


	DBConnector* dbConn = g_dbManager->Pop();
	DBBind<2, 0> dbBind(*dbConn, L"\
			UPDATE Player \
			SET Hp = (?) \
			WHERE PlayerDbId = (?);\
			");
	int32_t hp = stat.hp();
	if (hp == 0)
		hp = stat.max_hp();

	dbBind.BindParam(0, hp);
	dbBind.BindParam(1, playerDbId);
	bool success = dbBind.Execute();

	if (success)
	{
		room->PushTask([player]()
			{
				std::cout << "HP Saved : " << player->_statInfo->hp() << std::endl;
			});
	}
	g_dbManager->Push(dbConn);

}

void DBTransaction::RewardPlayer_Set(std::shared_ptr<Player> player, RewardData rewardData, std::shared_ptr<Room> room)
{
	if (player == nullptr || room == nullptr)
		return;

	auto slot = player->_inven->GetEmptySlot();
	if (slot.has_value() == false)
		return;

	PushTask(&DBTransaction::RewardPlayer_Proc,player, rewardData, room, slot.value());

}

void DBTransaction::RewardPlayer_Proc(std::shared_ptr<Player> player, RewardData rewardData, std::shared_ptr<Room> room,
	int32_t slot)
{
	if (player == nullptr || room == nullptr)
		return;

	int32_t TemplateId = rewardData.itemId;
	int32_t Count = rewardData.count;
	int32_t Slot = slot;
	int32_t OwnerDbId = player->_playerDbId;


	DBConnector* dbConn = g_dbManager->Pop();
	DBBind<4, 6> dbBind(*dbConn, L"\
	INSERT INTO Item (TemplateId, Count, Slot, OwnerDbId) \
	OUTPUT inserted.*\
	VALUES ( ?,?,?,?); \
	");


	dbBind.BindParam(0, TemplateId);
	dbBind.BindParam(1, Count);
	dbBind.BindParam(2, Slot);
	dbBind.BindParam(3, OwnerDbId);
	auto itemDb = std::make_shared<ItemDB>();
	dbBind.BindCol(0, itemDb->ItemDbId);
	dbBind.BindCol(1, itemDb->TemplateId);
	dbBind.BindCol(2, itemDb->Count);
	dbBind.BindCol(3, itemDb->Slot);
	dbBind.BindCol(4, itemDb->Equipped);
	dbBind.BindCol(5, itemDb->OwnerDbId);

	bool success = dbBind.Execute();

	if (success)
	{
		if (dbBind.Fetch())
		{
			room->PushTask([player, itemDb]()
				{
					std::shared_ptr<Item> newItem = Item::MakeItem(*itemDb);
					player->_inven->Add(newItem);
					{
						Protocol::MS_ADD_ITEM itemProto;
						auto itemInfo = itemProto.add_items();
						itemInfo->CopyFrom(*newItem->_itemInfo);

						auto sendBuffer = ClientProtocolManager::MakeSendBuffer(itemProto);

						if (auto session = player->_session.lock())
							session->Send(sendBuffer);
					}

				});
		}
	}
	g_dbManager->Push(dbConn);

	
}

void DBTransaction::EquipItemNoti_Set(std::shared_ptr<Player> player, std::shared_ptr<Item> item)
{
	if (player == nullptr || item == nullptr)
		return;

	PushTask(&DBTransaction::EquipItemNoti_Proc, player, item->_itemInfo->item_db_id(), item->_itemInfo->equipped());
}

void DBTransaction::EquipItemNoti_Proc(std::shared_ptr<Player> player, int32_t itemDbId, bool equipped)
{
	DBConnector* dbConn = g_dbManager->Pop();
	DBBind<2, 0> dbBind(*dbConn, L"\
	UPDATE Item \
	SET Equipped = (?)\
	WHERE ItemDbId = (?); \
	");

	dbBind.BindParam(0, equipped);
	dbBind.BindParam(1, itemDbId);

	bool success = dbBind.Execute();

	if (!success)
	{
		// ½ÇÆÐ?
	}
	g_dbManager->Push(dbConn);

}
