#include "pch.h"
#include "ObjectManager.h"
#include "Player.h"

ObjectManager g_objectManager;

int32_t ObjectManager::GenerateId(Protocol::ObjectType objectType)
{
	std::lock_guard<std::recursive_mutex> lock(_rMutex);

	return ((static_cast<int32_t>(objectType) << 24) | (_counter++));
}


bool ObjectManager::Remove(int32_t objectId)
{
	Protocol::ObjectType objectType = GetObjectTypeById(objectId);
	{
		std::lock_guard<std::recursive_mutex> lock(_rMutex);

		if (objectType == Protocol::ObjectType::PLAYER)
			return _players.erase(objectId);
	}

	return false;
}

std::shared_ptr<Player> ObjectManager::Find(int32_t objectId)
{
	Protocol::ObjectType objectType = GetObjectTypeById(objectId);

	{
		std::lock_guard<std::recursive_mutex> lock(_rMutex);
		if (objectType == Protocol::ObjectType::PLAYER)
		{
			auto it = _players.find(objectId);

			if (it != _players.end())
				return it->second;
		}

	}
	return nullptr;
}