#pragma once
#include "Protocol.pb.h"

class ItemDB;

class Item
{
public:
	Item(Protocol::ItemType itemType);
	~Item();

	static std::shared_ptr<Item> MakeItem(ItemDB& itemDb);
	


public:
	Protocol::ItemInfo* _itemInfo;
	Protocol::ItemType _itemType;
	bool _stackable = false;
};

class Weapon : public Item
{
public:
	Weapon(int32_t templateId);

	void Init(int32_t templateId);

public:
	Protocol::WeaponType _weaponType;
	int32_t _damage;
};

class Armor : public Item
{
public:
	Armor(int32_t templateId);

	void Init(int32_t templateId);

public:
	Protocol::ArmorType _armorType;
	int32_t _defence;
};

class Consumable : public Item
{
public:
	Consumable(int32_t templateId);

	void Init(int32_t templateId);

public:
	Protocol::ConsumableType _consumableType;
	int32_t _maxCount;
};
