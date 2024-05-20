#include "pch.h"
#include "Player.h"
#include "Item.h"

#include "DBTransaction.h"
#include "Inventory.h"
#include "VisualField.h"


Player::Player() : BaseObject(Protocol::ObjectType::PLAYER)
{
	_inven = std::make_shared<Inventory>();

	//TODO : ?
	//_visualField = std::make_shared<VisualField>(std::static_pointer_cast<Player>(shared_from_this()));
}

Player::~Player()
{
	std::cout << "~Player" << std::endl;
}

void Player::OnDamaged(std::shared_ptr<BaseObject> attacker, int32_t damage)
{
	BaseObject::OnDamaged(attacker, damage);
	
}

void Player::OnDead(std::shared_ptr<BaseObject> attacker)
{
	BaseObject::OnDead(attacker);
}

void Player::OnLeaveGame()
{
	std::shared_ptr<Player> player = std::static_pointer_cast<Player>(shared_from_this());
	std::shared_ptr<Room> room = _room.load().lock();
	g_dbTransaction->SavePlayerStatus_Set(player, room);
}

void Player::ManageEquipItem(Protocol::UC_EQUIP_ITEM equipProto)
{
	std::shared_ptr<Item> item = _inven->Get(equipProto.item_db_id());

	if (item == nullptr)
		return;

	if (item->_itemType == Protocol::ITEM_TYPE_CONSUMABLE)
		return;

	// 착용요청이라면 겹치는 부위 해제
	if (equipProto.equipped())
	{
		std::shared_ptr<Item> unequipItem = nullptr;
		if (item->_itemType == Protocol::ITEM_TYPE_WEAPON)
		{
			unequipItem = _inven->Find([](const std::shared_ptr<Item>& i)
				{
					return i->_itemInfo->equipped() && i->_itemType == Protocol::ITEM_TYPE_WEAPON;
				});

		}
		else if (item->_itemType == Protocol::ITEM_TYPE_ARMOR)
		{
			Protocol::ArmorType armorType = std::static_pointer_cast<Armor>(item)->_armorType;
			unequipItem = _inven->Find([armorType](const std::shared_ptr<Item>& i)
				{
					return i->_itemInfo->equipped() && i->_itemType == Protocol::ITEM_TYPE_ARMOR
						&& std::static_pointer_cast<Armor>(i)->_armorType == armorType;
				});
		}

		if (unequipItem != nullptr)
		{
			// 메모리
			unequipItem->_itemInfo->set_equipped(false);

			// DB
			g_dbTransaction->EquipItemNoti_Set(std::static_pointer_cast<Player>(shared_from_this()), unequipItem);

			// TO클라
			Protocol::MS_EQUIP_ITEM equipOkItem;
			equipOkItem.set_item_db_id(unequipItem->_itemInfo->item_db_id());
			equipOkItem.set_equipped(unequipItem->_itemInfo->equipped());

			auto sendBuffer = ClientProtocolManager::MakeSendBuffer(equipOkItem);
			if (auto session = _session.lock())
				session->Send(sendBuffer);
		}
	}
	{
		// 메모리
		item->_itemInfo->set_equipped(equipProto.equipped());

		// DB
		g_dbTransaction->EquipItemNoti_Set(std::static_pointer_cast<Player>(shared_from_this()), item);

		// TO클라
		Protocol::MS_EQUIP_ITEM equipOkItem;
		equipOkItem.set_item_db_id(equipProto.item_db_id());
		equipOkItem.set_equipped(equipProto.equipped());

		auto sendBuffer = ClientProtocolManager::MakeSendBuffer(equipOkItem);
		if (auto session = _session.lock())
			session->Send(sendBuffer);
	}

	UpdateAdditionalStat();
}

void Player::UpdateAdditionalStat()
{
	_weaponDamage = 0;
	_armorDefence = 0;

	for (auto item : _inven->_items)
	{
		if (item.second->_itemInfo->equipped() == false)
			continue;

		switch (item.second->_itemType)
		{
		case Protocol::ITEM_TYPE_WEAPON:
			_weaponDamage += std::static_pointer_cast<Weapon>(item.second)->_damage;
			break;
		case Protocol::ITEM_TYPE_ARMOR:
			_armorDefence += std::static_pointer_cast<Armor>(item.second)->_defence;
			break;
		}
	}
}
