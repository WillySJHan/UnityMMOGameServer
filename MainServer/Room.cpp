#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "ClientSession.h"
#include "Protocol.pb.h"
#include "Map.h"
#include "Bullet.h"
#include "DataManager.h"
#include "DBTransaction.h"
#include "Monster.h"
#include "Projectile.h"
#include "Inventory.h"
#include "Item.h"
#include "Area.h"
#include "VisualField.h"

Room::Room() : _map(/*std::make_shared<Map>()*/)
{
}

void Room::Init(int32_t mapId, int32_t areaCells)
{
	_map->LoadMap(mapId);

	_areaCells = areaCells;

	int32_t countY = (_map->SizeY() + areaCells - 1) / areaCells;
	int32_t countX = (_map->SizeX() + areaCells - 1) / areaCells;

	_areas = std::vector<std::vector<std::shared_ptr<Area>>>(countY, std::vector<std::shared_ptr<Area>>(countX));

	for (int32_t y = 0; y < countY; y++)
	{
		for (int32_t x = 0; x < countX; x++)
		{
			_areas[y][x] = std::make_shared<Area>(y, x);
		}
	}
	// 몬스터 생성
	for (int32_t i = 0; i < 500; i++)
	{
		std::shared_ptr<Monster> monster = g_objectManager.Add<Monster>();
		monster->Init(1);
		EnterGame(monster, true);
	}
}

void Room::EnterGame(std::shared_ptr<BaseObject> baseObject, bool randomPos)
{
	if (baseObject == nullptr)
		return;

	if (EnterObject(baseObject, randomPos) == false)
		return;

	Protocol::ObjectType type = ObjectManager::GetObjectTypeById(baseObject->_objectInfo->object_id());

	if (type == Protocol::ObjectType::PLAYER)
	{
		// 자신한테 전송
		{
			std::shared_ptr<Player> player = std::static_pointer_cast<Player>(baseObject);

			Protocol::MS_ENTER_GAME enterProto;
			enterProto.mutable_player()->CopyFrom(*player->_objectInfo);
			auto sendBuffer = ClientProtocolManager::MakeSendBuffer(enterProto);

			if (auto session = player->_session.lock())
				session->Send(sendBuffer);

			auto visualField = player->_visualField;

			PushTask([visualField]()
			{
				 visualField->Update();
			});

		}
	}

	// 타인들한테 전송
	{
		Protocol::MS_SPAWN spawnProto;
		auto playerInfo = spawnProto.add_objects();
		playerInfo->CopyFrom(*baseObject->_objectInfo);

		std::shared_ptr<SendBuffer> sendBuffer = ClientProtocolManager::MakeSendBuffer(spawnProto);
		Broadcast(baseObject->GetCellPos(), sendBuffer);
	}

}

void Room::LeaveGame(std::shared_ptr<BaseObject> baseObject)
{
	if (baseObject == nullptr)
		return;

	const int32_t objectId = baseObject->_objectInfo->object_id();
	if (LeaveObject(objectId) == false)
		return;

	Protocol::ObjectType type = ObjectManager::GetObjectTypeById(objectId);

	if (type == Protocol::ObjectType::PLAYER)
	{
		// 본인한테 전송
		{
			std::shared_ptr<Player> player = std::static_pointer_cast<Player>(baseObject);
			Protocol::MS_LEAVE_GAME leaveProto;
			std::shared_ptr<SendBuffer> sendBuffer = ClientProtocolManager::MakeSendBuffer(leaveProto);
			if (auto session = player->_session.lock())
				session->Send(sendBuffer);
		}
	}


	// 타인들한테 전송
	{
		Protocol::MS_DESPAWN despawnProto;
		despawnProto.add_object_ids(objectId);
		std::shared_ptr<SendBuffer> sendBuffer = ClientProtocolManager::MakeSendBuffer(despawnProto);
		
		Broadcast(baseObject->GetCellPos(), sendBuffer);
	}
}

void Room::ManageMove(std::shared_ptr<Player> player, Protocol::UC_MOVE moveProto)
{
	if (player == nullptr)
		return;
	// TODO Validation

	Protocol::PositionInfo* movePosInfo = moveProto.mutable_pos_info();
	Protocol::PositionInfo* posInfo = player->_posInfo;

	// CanGo?
	if (movePosInfo->pos_x() != posInfo->pos_x() || movePosInfo->pos_y() != posInfo->pos_y())
	{
		if (_map->CanGo(Vector2Int(movePosInfo->pos_x(), movePosInfo->pos_y())) == false)
			return;
	}
	posInfo->set_state(movePosInfo->state());
	posInfo->set_move_dir(movePosInfo->move_dir());
	_map->ApplyMove(player, Vector2Int(movePosInfo->pos_x(), movePosInfo->pos_y()));

	// 타인들에게 전송
	{
		Protocol::MS_MOVE resMoveProto;
		resMoveProto.set_object_id(player->_objectInfo->object_id());
		resMoveProto.mutable_pos_info()->CopyFrom(moveProto.pos_info());

		auto sendBuffer = ClientProtocolManager::MakeSendBuffer(resMoveProto);

		Broadcast(player->GetCellPos(), sendBuffer);
	}
}

void Room::ManageSkill(std::shared_ptr<Player> player, Protocol::UC_SKILL skillProto)
{
	if (player == nullptr)
		return;
	
	{
		Protocol::ObjectInfo* info = player->_objectInfo;
		if (info->mutable_pos_info()->state() != Protocol::CreatureState::IDLE)
			return;

		info->mutable_pos_info()->set_state(Protocol::CreatureState::SKILL);
		Protocol::MS_SKILL skill;
		skill.set_object_id(info->object_id());
		skill.mutable_info()->set_skill_id(skillProto.mutable_info()->skill_id());

		auto sendBuffer = ClientProtocolManager::MakeSendBuffer(skill);
		Broadcast(player->GetCellPos(), sendBuffer);

		Skill skillData = DataManager::SkillMap[skillProto.mutable_info()->skill_id()];
		switch (skillData.skillType)
		{
		case Protocol::SkillType::SKILL_AUTO:
			{
				Vector2Int skillPos = player->GetFrontCellPos(info->mutable_pos_info()->move_dir());
				std::shared_ptr<BaseObject> target = _map->Find(skillPos);
				if (target != nullptr)
				{
					std::cout << "hit BaseObject" << std::endl;
				}
			}
			break;
		case Protocol::SkillType::SKILL_PROJECTILE:
			{
				std::shared_ptr<Bullet> bullet = g_objectManager.Add<Bullet>();
				if (bullet == nullptr)
					return;

				bullet->_owner = player;
				*bullet->_data = skillData;
				bullet->_posInfo->set_state(Protocol::CreatureState::MOVING);
				bullet->_posInfo->set_move_dir(player->_posInfo->move_dir());
				bullet->_posInfo->set_pos_x(player->_posInfo->pos_x());
				bullet->_posInfo->set_pos_y(player->_posInfo->pos_y());
				bullet->_statInfo->set_speed(skillData.projectile.speed);

				EnterGame(bullet, false);
			}
			break;
		}
	}
}

void Room::Broadcast(Vector2Int pos, std::shared_ptr<SendBuffer> sendBuffer)
{
	// TODO : 먼저 벡터에 뽑아서
	// TODO &
	auto areas = GetNearAreas(pos);
	for (auto area : areas)
	{
		for (auto p : area->_players)
		{
			int32_t dx = p->GetCellPos().x - pos.x;
			int32_t dy = p->GetCellPos().y - pos.y;

			if (std::abs(dx) > Room::VisualFieldCells)
				continue;
			if (std::abs(dy) > Room::VisualFieldCells)
				continue;
			if (auto session = p->_session.lock())
				session->Send(sendBuffer);
		}
	}
}

void Room::ChangeRoom(std::shared_ptr<Player> player, Protocol::UC_CHANGE_ROOM changeRoomProto)
{

	auto baseObject = std::static_pointer_cast<BaseObject>(player);
	LeaveGame(baseObject);

	player->_posInfo->set_state(Protocol::CreatureState::IDLE);
	player->_posInfo->set_move_dir(Protocol::MoveDir::DOWN);

	
	if (player->_visualField->_task != nullptr)
	{
		player->_visualField->_task->_cancel = true;
		player->_visualField->_task = nullptr;
	}
	player->_visualField->_previousObjects.clear();

	g_roomManager->Find(changeRoomProto.room_id())->PushTask(&Room::EnterGame, baseObject, true);
}

void Room::ManageEquipItem(std::shared_ptr<Player> player, Protocol::UC_EQUIP_ITEM equipProto)
{
	if (player == nullptr)
		return;

	player->ManageEquipItem(equipProto);
}

std::shared_ptr<Area> Room::GetArea(Vector2Int cellPos)
{
	int32_t x = (cellPos.x - _map->_minX) / _areaCells;
	int32_t y = (_map->_maxY - cellPos.y) / _areaCells;

	return GetArea(y, x);
}

std::shared_ptr<Area> Room::GetArea(int32_t indexY, int32_t indexX)
{
	if (indexX < 0 || indexX >= _areas[1].size())
		return nullptr;
	if (indexY < 0 || indexY >= _areas.size())
		return nullptr;

	return _areas[indexY][indexX];
}

std::shared_ptr<Player> Room::FindNearPlayer(Vector2Int pos, int32_t searchRange)
{
	std::vector <std::shared_ptr<Player>> players = GetNearPlayers(pos, searchRange);

	std::sort(players.begin(), players.end(), [&](std::shared_ptr<Player> left, std::shared_ptr<Player> right)
		{
			int32_t leftDist = (left->GetCellPos() - pos).GetCellDistFromZero();
			int32_t rightDist = (right->GetCellPos() - pos).GetCellDistFromZero();
			return leftDist < rightDist;
		});

	for (auto player : players)
	{
		std::vector<Vector2Int> path = _map->FindPath(pos, player->GetCellPos(), true);
		if (path.size() < 2 || path.size() > searchRange)
			continue;

		return player;
	}

	return nullptr;
}

std::vector<std::shared_ptr<Player>> Room::GetNearPlayers(Vector2Int pos, int32_t searchRange)
{
	std::vector<std::shared_ptr<Area>> areas = GetNearAreas(pos, searchRange);
	std::vector<std::shared_ptr<Player>> players;
	for (auto area : areas)
	{
		for (auto player : area->_players)
		{
			players.push_back(player);
		}
	}
	return players;
}

std::vector<std::shared_ptr<Area>> Room::GetNearAreas(Vector2Int cellPos, int32_t searchRange)
{
	std::unordered_set<std::shared_ptr<Area>> areas;
	int32_t maxY = cellPos.y + searchRange;
	int32_t minY = cellPos.y - searchRange;
	int32_t maxX = cellPos.x + searchRange;
	int32_t minX = cellPos.x - searchRange;

	//좌측상단
	Vector2Int leftTop(minX, maxY);
	int32_t minIndexY = (_map->_maxY - leftTop.y) / _areaCells;
	int32_t minIndexX = (leftTop.x - _map->_minX) / _areaCells;
	//우측하단
	Vector2Int rightBot(maxX, minY);
	int32_t maxIndexY = (_map->_maxY - rightBot.y) / _areaCells;
	int32_t maxIndexX = (rightBot.x - _map->_minX) / _areaCells;

	for (int32_t x = minIndexX; x <= maxIndexX; x++)
	{
		for (int32_t y = minIndexY; y <= maxIndexY; y++)
		{
			std::shared_ptr<Area> area = GetArea(y, x);
			if (area == nullptr)
				continue;

			areas.insert(area);
		}
	}
	std::vector<std::shared_ptr<Area>> vArea(areas.begin(), areas.end());
	return vArea;
}

std::shared_ptr<Player> Room::FindPlayer(std::function<bool(std::shared_ptr<BaseObject>)> condition)
{
	// TODO 반복문 조심 
	for (const auto/*&*/ pair : _players)
	{
		if (condition(pair.second))
			return pair.second;
	}

	return nullptr;
}

bool Room::EnterObject(std::shared_ptr<BaseObject> baseObject, bool randomPos)
{

	if (randomPos)
	{
		Vector2Int respawnPos(0, 0);

		if (_id == 1)
		{
			while (true)
			{
				respawnPos.x = Utility::GetRandom(_map->_minX, _map->_maxX-125);
				respawnPos.y = Utility::GetRandom(_map->_minY, _map->_maxY);
				if (_map->Find(respawnPos) == nullptr)
				{
					baseObject->SetCellpos(respawnPos);
					break;
				}
			}
		}
		else if (_id == 2)
		{
			while (true)
			{
				respawnPos.x = Utility::GetRandom(_map->_minX + 125, _map->_maxX);
				respawnPos.y = Utility::GetRandom(_map->_minY, _map->_maxY);
				if (_map->Find(respawnPos) == nullptr)
				{
					baseObject->SetCellpos(respawnPos);
					break;
				}
			}
		}
	}


	Protocol::ObjectType type = ObjectManager::GetObjectTypeById(baseObject->_objectInfo->object_id());

	if (type == Protocol::ObjectType::PLAYER)
	{
		std::shared_ptr<Player> player = std::static_pointer_cast<Player>(baseObject);

		// 있다면 문제가 있다
		if (_players.find(player->_objectInfo->object_id()) != _players.end())
			return false;

		_players.insert(std::make_pair(player->_objectInfo->object_id(), player));

		player->_room.store(std::static_pointer_cast<Room>(shared_from_this()));

		player->UpdateAdditionalStat();
		_map->ApplyMove(player, Vector2Int(player->_posInfo->pos_x(), player->_posInfo->pos_y()));
		GetArea(player->GetCellPos())->_players.insert(player);

		return true;
	}
	else if (type == Protocol::ObjectType::MONSTER)
	{
		std::shared_ptr<Monster> monster = std::static_pointer_cast<Monster>(baseObject);

		// 있다면 문제가 있다
		if (_monsters.find(monster->_objectInfo->object_id()) != _monsters.end())
			return false;

		_monsters.insert(std::make_pair(monster->_objectInfo->object_id(), monster));

		monster->_room.store(std::static_pointer_cast<Room>(shared_from_this()));

		GetArea(monster->GetCellPos())->_monsters.insert(monster);
		_map->ApplyMove(monster, Vector2Int(monster->_posInfo->pos_x(), monster->_posInfo->pos_y()));

		PushTask([monster]()
		{
			monster->Update();
		});
						

		return true;
	}
	else if (type == Protocol::ObjectType::PROJECTILE)
	{
		std::shared_ptr<Projectile> projectile = std::static_pointer_cast<Projectile>(baseObject);

		// 있다면 문제가 있다
		if (_projectiles.find(projectile->_objectInfo->object_id()) != _projectiles.end())
			return false;

		_projectiles.insert(std::make_pair(projectile->_objectInfo->object_id(), projectile));

		projectile->_room.store(std::static_pointer_cast<Room>(shared_from_this()));

		GetArea(projectile->GetCellPos())->_projectiles.insert(projectile);

		PushTask([projectile]()
		{
				projectile->Update();
		});

		return true;
	}

	return false;
}

bool Room::LeaveObject(int32_t objectId)
{
	Protocol::ObjectType type = ObjectManager::GetObjectTypeById(objectId);

	if (type == Protocol::ObjectType::PLAYER)
	{
		// 없다면 문제가 있다.
		if (_players.find(objectId) == _players.end())
			return false;

		std::shared_ptr<Player> player = _players[objectId];


		// DB저장
		player->OnLeaveGame();

		_map->ApplyLeave(player);
		player->_room.store(std::weak_ptr<Room>());

		_players.erase(objectId);

		return true;
	}
	else if (type == Protocol::ObjectType::MONSTER)
	{
		// 없다면 문제가 있다.
		if (_monsters.find(objectId) == _monsters.end())
			return false;

		std::shared_ptr<Monster> monster = _monsters[objectId];

		_map->ApplyLeave(monster);
		monster->_room.store(std::weak_ptr<Room>());
		_monsters.erase(objectId);

		return true;
	}
	else if (type == Protocol::ObjectType::PROJECTILE)
	{
		// 없다면 문제가 있다.
		if (_projectiles.find(objectId) == _projectiles.end())
			return false;

		std::shared_ptr<Projectile> projectile = _projectiles[objectId];
		_map->ApplyLeave(projectile);
		projectile->_room.store(std::weak_ptr<Room>());

		_projectiles.erase(objectId);
		
		return true;
	}

	return false;
}


