#pragma once


class Item;

class Inventory
{
public:

	~Inventory();
	void Add(std::shared_ptr<Item> item);
	std::shared_ptr<Item> Get(int32_t itemDbId);
	std::shared_ptr<Item> Find(std::function<bool(std::shared_ptr<Item>)> condition);

	std::optional<int32_t> GetEmptySlot();


public:
	std::unordered_map<int32_t, std::shared_ptr<Item>> _items;
};

