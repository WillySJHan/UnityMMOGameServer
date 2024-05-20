#include "pch.h"
#include "VisualField.h"
#include "Area.h"
#include "ClientProtocolManager.h"
#include "Player.h"
#include "Room.h"
#include "Monster.h"
#include "Projectile.h"
#include "ClientSession.h"
#include <optional>


VisualField::VisualField(std::shared_ptr<Player> owner) : _owner(owner)
{
}

VisualField::~VisualField()
{
	std::cout << "~VisualField" << std::endl;
}

std::optional<std::unordered_set<std::shared_ptr<BaseObject>>> VisualField::SurroundingObjects()
{
	auto owner = _owner.lock();
	if (owner == nullptr)
		return std::nullopt;
	auto room = owner->_room.load().lock();
	if (room == nullptr)
		return std::nullopt;

	std::unordered_set<std::shared_ptr<BaseObject>> objects;

	Vector2Int cellPos = owner->GetCellPos();
	std::vector<std::shared_ptr<Area>> areas = room->GetNearAreas(cellPos);

	for (auto area : areas)
	{
		for (auto player : area->_players)
		{
			int dx = player->GetCellPos().x - cellPos.x;
			int dy = player->GetCellPos().y - cellPos.y;

			if (std::abs(dx) > Room::VisualFieldCells)
				continue;
			if (std::abs(dy) > Room::VisualFieldCells)
				continue;

			objects.insert(player);
		}
		for (auto monster : area->_monsters)
		{
			int dx = monster->GetCellPos().x - cellPos.x;
			int dy = monster->GetCellPos().y - cellPos.y;

			if (std::abs(dx) > Room::VisualFieldCells)
				continue;
			if (std::abs(dy) > Room::VisualFieldCells)
				continue;

			objects.insert(monster);
		}
		for (auto projectile : area->_projectiles)
		{
			int dx = projectile->GetCellPos().x - cellPos.x;
			int dy = projectile->GetCellPos().y - cellPos.y;

			if (std::abs(dx) > Room::VisualFieldCells)
				continue;
			if (std::abs(dy) > Room::VisualFieldCells)
				continue;

			objects.insert(projectile);
		}
	}

	return objects;
}

void VisualField::Update()
{
	auto owner = _owner.lock();
	if (owner == nullptr)
		return;
	auto room = owner->_room.load().lock();
	if (room == nullptr)
		return;

	std::optional<std::unordered_set<std::shared_ptr<BaseObject>>> value = SurroundingObjects();
	std::unordered_set<std::shared_ptr<BaseObject>> currentObjects;
	if (value.has_value())
		currentObjects = value.value();

	//spawn

	// TODO : 복사
	std::vector<std::shared_ptr<BaseObject>> previousObjects(_previousObjects.begin(), _previousObjects.end());
	std::unordered_set< std::shared_ptr<BaseObject>> difference(currentObjects.begin(), currentObjects.end());
	for (const auto& value : previousObjects)
	{
		difference.erase(value);
	}
	// TODO : 복사
	//std::vector<std::shared_ptr<BaseObject>> added(difference.begin(), difference.end());
	if (difference.size() > 0)
	{
		Protocol::MS_SPAWN spawnProto;

		for (auto& baseObject : difference)
		{
			auto info = spawnProto.add_objects();
			info->CopyFrom(*baseObject->_objectInfo);
		}

		auto sendBuffer = ClientProtocolManager::MakeSendBuffer(spawnProto);
		if (auto session = owner->_session.lock())
			session->Send(sendBuffer);
	}
	//despawn
	std::unordered_set< std::shared_ptr<BaseObject>> difference2(_previousObjects.begin(), _previousObjects.end());
	for (const auto& value : currentObjects)
	{
		difference2.erase(value);
	}
	//std::vector<std::shared_ptr<BaseObject>> removed(difference2.begin(), difference2.end());
	if (difference2.size() > 0)
	{
		Protocol::MS_DESPAWN despawnProto;

		for (auto& baseObject : difference2)
		{
			despawnProto.add_object_ids(baseObject->_objectInfo->object_id());
		}
		auto sendBuffer = ClientProtocolManager::MakeSendBuffer(despawnProto);
		if (auto session = owner->_session.lock())
			session->Send(sendBuffer);
	}

	_previousObjects = currentObjects;


	auto visualField = shared_from_this();
	_task = room->ReservePush(200,[visualField]()
	{
			visualField->Update();
	});

}