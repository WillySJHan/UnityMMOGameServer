#include "pch.h"
#include "Item.h"

#include "DataManager.h"
#include "DBTransaction.h"

Item::Item(Protocol::ItemType itemType) : _itemType(itemType)
{
	_itemInfo = new Protocol::ItemInfo();
}

Item::~Item()
{
	std::cout << "~Item" << std::endl;
}

std::shared_ptr<Item> Item::MakeItem(ItemDB& itemDb)
{
	std::shared_ptr<Item> item;

	ItemData* itemData;
	auto it = DataManager::ItemMap.find(itemDb.TemplateId);
	if (it != DataManager::ItemMap.end())
		itemData = it->second;
	else
		return nullptr;

	switch (itemData->itemType)
	{
	case Protocol::ITEM_TYPE_WEAPON:
		item = std::make_shared<Weapon>(itemDb.TemplateId);
		break;
	case Protocol::ITEM_TYPE_ARMOR:
		item = std::make_shared<Armor>(itemDb.TemplateId);
		break;
	case Protocol::ITEM_TYPE_CONSUMABLE:
		item = std::make_shared<Consumable>(itemDb.TemplateId);
		break;
	}

	if (item != nullptr)
	{
		item->_itemInfo->set_count(itemDb.Count);
		item->_itemInfo->set_item_db_id(itemDb.ItemDbId);
		item->_itemInfo->set_slot(itemDb.Slot);
		item->_itemInfo->set_equipped(itemDb.Equipped);
	}

	return item;
}

Weapon::Weapon(int32_t templateId) : Item(Protocol::ITEM_TYPE_WEAPON)
{
	Init(templateId);
}

void Weapon::Init(int32_t templateId)
{
	ItemData* itemData;
	auto it = DataManager::ItemMap.find(templateId);
	if (it != DataManager::ItemMap.end())
		itemData = it->second;
	else
		return;

	if (itemData->itemType != Protocol::ITEM_TYPE_WEAPON)
		return;

	WeaponData* data = static_cast<WeaponData*>(itemData);
	{
		_itemInfo->set_template_id(data->id);
		_itemInfo->set_count(1);
		_weaponType = data->weaponType;
		_damage = data->damage;
		_stackable = false;
	}
}

Armor::Armor(int32_t templateId) : Item(Protocol::ITEM_TYPE_ARMOR)
{
	Init(templateId);
}

void Armor::Init(int32_t templateId)
{
	ItemData* itemData;
	auto it = DataManager::ItemMap.find(templateId);
	if (it != DataManager::ItemMap.end())
		itemData = it->second;
	else
		return;

	if (itemData->itemType != Protocol::ITEM_TYPE_ARMOR)
		return;

	ArmorData* data = static_cast<ArmorData*>(itemData);
	{
		_itemInfo->set_template_id(data->id);
		_itemInfo->set_count(1);
		_armorType = data->armorType;
		_defence = data->defence;
		_stackable = false;
	}
}

Consumable::Consumable(int32_t templateId) : Item(Protocol::ITEM_TYPE_CONSUMABLE)
{
	Init(templateId);
}

void Consumable::Init(int32_t templateId)
{
	ItemData* itemData;
	auto it = DataManager::ItemMap.find(templateId);
	if (it != DataManager::ItemMap.end())
		itemData = it->second;
	else
		return;

	if (itemData->itemType != Protocol::ITEM_TYPE_CONSUMABLE)
		return;

	ConsumableData* data = static_cast<ConsumableData*>(itemData);
	{
		_itemInfo->set_template_id(data->id);
		_itemInfo->set_count(1);
		_consumableType = data->consumableType;
		_maxCount = data->maxCount;
		_stackable = (data->maxCount>1);
	}
}
