#include "pch.h"
#include "Inventory.h"

#include "Item.h"
#include <optional>

Inventory::~Inventory()
{
	std::cout << "~Inven" << std::endl;
}

void Inventory::Add(std::shared_ptr<Item> item)
{
	_items[item->_itemInfo->item_db_id()] = item;
}

std::shared_ptr<Item> Inventory::Get(int32_t itemDbId)
{
	auto it = _items.find(itemDbId);

	if (it != _items.end())
		return it->second;
	else
		return nullptr;
}

std::shared_ptr<Item> Inventory::Find(std::function<bool(std::shared_ptr<Item>)> condition)
{
	for (auto item : _items)
	{
		if(condition(item.second))
		{
			return item.second;
		}
	}

	return nullptr;
}

std::optional<int32_t> Inventory::GetEmptySlot()
{
	// TODO vector¿¡ »Ì¾Æ¼­?
	for (int32_t slot = 0;slot<20;slot++)
	{
		auto it = std::find_if(_items.begin(), _items.end(),
			[slot](const std::pair<int, std::shared_ptr<Item>>& pair) {
				return pair.second->_itemInfo->slot() == slot;
			});

		if (it == _items.end())
			return slot;
	}
	return std::nullopt;
}
