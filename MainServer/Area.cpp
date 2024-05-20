#include "pch.h"
#include "Area.h"

#include "ObjectManager.h"
#include "Player.h"
#include "Monster.h"
#include "Projectile.h"

Area::Area(int32_t y, int32_t x) : _indexY(y), _indexX(x)
{

}

std::shared_ptr<Player> Area::FindOnePlayer(std::function<bool(std::shared_ptr<Player>)> condition)
{
	for (auto player : _players)
	{
		if (condition(player))
			return player;
	}

	return nullptr;
}

std::vector<std::shared_ptr<Player>> Area::FindAllPlayers(std::function<bool(std::shared_ptr<Player>)> condition)
{
	std::vector<std::shared_ptr<Player>> findList;

	for (auto player : _players)
	{
		if (condition(player))
			findList.push_back(player);
	}

	return findList;
}

void Area::Remove(std::shared_ptr<BaseObject> baseObject)
{
	Protocol::ObjectType type = ObjectManager::GetObjectTypeById(baseObject->_objectInfo->object_id());

	switch (type)
	{
	case Protocol::ObjectType::PLAYER:
		_players.erase(std::static_pointer_cast<Player>(baseObject));
		break;
	case Protocol::ObjectType::MONSTER:
		_monsters.erase(std::static_pointer_cast<Monster>(baseObject));
		break;
	case Protocol::ObjectType::PROJECTILE:
		_projectiles.erase(std::static_pointer_cast<Projectile>(baseObject));
		break;

	}
}