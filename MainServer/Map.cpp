#include "pch.h"
#include "Map.h"

#include "Area.h"
#include "Player.h"
#include "Room.h"
#include "Monster.h"
#include "Projectile.h"


bool Map::CanGo(Vector2Int cellPos, bool checkObjects)
{
	if (cellPos.x < _minX || cellPos.x > _maxX)
		return false;
	if (cellPos.y < _minY || cellPos.y > _maxY)
		return false;

	int x = cellPos.x - _minX;
	int y = _maxY - cellPos.y;
	return (_collision[y][x] == 0 && (!checkObjects || _objects[y][x] == nullptr));
}

void Map::LoadMap(int32_t mapId, const std::string& pathPrefix)
{
	std::string a = pathPrefix + "/Map_" + std::string(3 - std::to_string(mapId).length(), '0') + std::to_string(mapId) + ".txt";
	std::ifstream file(a);
	if (!file)
	{
		return;
	}
	file >> _minX >> _maxX >> _minY >> _maxY;

	int32_t xCount = _maxX - _minX + 1;
	int32_t yCount = _maxY - _minY + 1;

	_collision = std::vector<std::vector<int32_t>>(yCount, std::vector<int32_t>(xCount, 0));
	_objects = std::vector<std::vector<std::shared_ptr<BaseObject>>>(yCount, std::vector<std::shared_ptr<BaseObject>>(xCount, nullptr));

	for (int y = 0; y < yCount; ++y) {
		std::string line;
		file >> line;
		for (int x = 0; x < xCount; ++x) {
			//_collision[y][x] = (line[x] == '1' ? true : false);
			_collision[y][x] = line[x] - '0';
		}
	}
}

std::shared_ptr<BaseObject> Map::Find(Vector2Int cellPos)
{
	if (cellPos.x < _minX || cellPos.x > _maxX)
		return nullptr;
	if (cellPos.y < _minY || cellPos.y > _maxY)
		return nullptr;

	int x = cellPos.x - _minX;
	int y = _maxY - cellPos.y;
	return _objects[y][x];
}

bool Map::ApplyMove(std::shared_ptr<BaseObject> baseObject, Vector2Int dest, bool checkObjects, bool collision)
{
	auto room = baseObject->_room.load().lock();

	if (room == nullptr)
		return false;
	
	if (room->_map != shared_from_this())
		return false;

	Protocol::PositionInfo* posInfo = baseObject->_posInfo;

	if (CanGo(dest, checkObjects) == false)
		return false;

	if (collision)
	{
		{
			int32_t x = posInfo->pos_x() - _minX;
			int32_t y = _maxY - posInfo->pos_y();

			if (_objects[y][x] == baseObject)
				_objects[y][x] = nullptr;
		}
		{
			int32_t x = dest.x - _minX;
			int32_t y = _maxY - dest.y;
			_objects[y][x] = baseObject;
		}
	}

	//Area
	Protocol::ObjectType type = ObjectManager::GetObjectTypeById(baseObject->_objectInfo->object_id());
	if (type == Protocol::ObjectType::PLAYER)
	{
		std::shared_ptr<Player> p = std::static_pointer_cast<Player>(baseObject);

		std::shared_ptr<Area> now = room->GetArea(baseObject->GetCellPos());
		std::shared_ptr<Area> after = room->GetArea(dest);
		if (now != after)
		{
			now->_players.erase(p);
			after->_players.insert(p);
		}
	}
	else if (type == Protocol::ObjectType::MONSTER)
	{
		std::shared_ptr<Monster> m = std::static_pointer_cast<Monster>(baseObject);

		std::shared_ptr<Area> now = room->GetArea(baseObject->GetCellPos());
		std::shared_ptr<Area> after = room->GetArea(dest);
		if (now != after)
		{
			now->_monsters.erase(m);
			after->_monsters.insert(m);
		}
	}
	else if (type == Protocol::ObjectType::PROJECTILE)
	{
		std::shared_ptr<Projectile> p = std::static_pointer_cast<Projectile>(baseObject);

		std::shared_ptr<Area> now = room->GetArea(baseObject->GetCellPos());
		std::shared_ptr<Area> after = room->GetArea(dest);
		if (now != after)
		{
			now->_projectiles.erase(p);
			after->_projectiles.insert(p);
		}
	}

	// 좌표이동
	posInfo->set_pos_x(dest.x);
	posInfo->set_pos_y(dest.y);

	return true;
}

bool Map::ApplyLeave(std::shared_ptr<BaseObject> baseObject)
{
	auto room = baseObject->_room.load().lock();

	if (room == nullptr)
		return false;
	
	if (room->_map != shared_from_this())
		return false;


	Protocol::PositionInfo* posInfo = baseObject->_posInfo;

	if (posInfo->pos_x() < _minX || posInfo->pos_x() > _maxX)
		return false;
	if (posInfo->pos_y() < _minY || posInfo->pos_y() > _maxY)
		return false;


	// Area
	std::shared_ptr<Area> area = room->GetArea(baseObject->GetCellPos());
	area->Remove(baseObject);

	{
		int x = posInfo->pos_x() - _minX;
		int y = _maxY - posInfo->pos_y();

		if (_objects[y][x] == baseObject)
			_objects[y][x] = nullptr;
	}

	return true;
}

std::vector<Vector2Int> Map::FindPath(Vector2Int startCellPos, Vector2Int destCellPos, bool checkObjects, int32_t maxDist)
{
	// A* 개선
	std::vector<Pos> path;

	std::unordered_set<Pos> closeList;
	std::unordered_map<Pos, int32_t> openList;

	std::unordered_map<Pos, Pos> parent;

	std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> pq;

	Pos pos = CellToPos(startCellPos);
	Pos dest = CellToPos(destCellPos);

	// 시작점 발견 예약 진행
	openList[pos] = 10 * (std::abs(dest.Y - pos.Y) + std::abs(dest.X - pos.X));

	pq.push(PQNode{ 10 * (std::abs(dest.Y - pos.Y) + std::abs(dest.X - pos.X)),0,pos.Y,pos.X });
	parent[pos] = pos;

	while (pq.empty() == false)
	{
		// 제일 좋은 후보를 찾는다

		PQNode pqNode = pq.top();
		pq.pop();

		Pos node{ pqNode.Y,pqNode.X };
		// 동일한 좌표를 여러 경로로 찾아서, 더 빠른 경로로 인해서 이미 방문(closed)된 경우 스킵
		if (closeList.find(node) != closeList.end())
			continue;

		// 방문한다
		closeList.insert(node);
		// 목적지 도착했으면 바로 종료
		if (node.Y == dest.Y && node.X == dest.X)
			break;

		// 상하좌우 등 이동할 수 있는 좌표인지 확인해서 예약(open)한다
		for (int32_t i = 0; i < _deltaY.size(); i++)
		{
			Pos next = Pos{ node.Y + _deltaY[i], node.X + _deltaX[i] };

			// 너무 멀면 스킵
			if (std::abs(pos.Y - next.Y) + std::abs(pos.X - next.X) > maxDist)
				continue;

			// 유효 범위를 벗어났으면 스킵
			// 벽으로 막혀서 갈 수 없으면 스킵
			if (next.Y != dest.Y || next.X != dest.X)
			{
				if (CanGo(PosToCell(next), checkObjects) == false) // CellPos
					continue;
			}

			// 이미 방문한 곳이면 스킵
			if (closeList.find(next) != closeList.end())
				continue;

			// 비용 계산
			int32_t g = 0;// node.G + _cost[i];
			int32_t h = 10 * ((dest.Y - next.Y) * (dest.Y - next.Y) + (dest.X - next.X) * (dest.X - next.X));
			// 다른 경로에서 더 빠른 길 이미 찾았으면 스킵

			int32_t value = 0;
			auto it = openList.find(next);

			if (it == openList.end())
			{
				value = INT32_MAX;
			}
			else
			{
				value = it->second;
			}

			if (value < g + h)
				continue;

			// 예약 진행

			auto result = openList.insert({ next, g + h });

			if (!result.second) 
				openList[next] = g + h;
			
			pq.push(PQNode{ g + h,g,next.Y,next.X });

			auto result2 = parent.insert({ next, node });

			if (!result2.second)
				parent[next] = node;
			
		}
	}

	return CalcCellPathFromParent(parent, dest);
}

std::vector<Vector2Int> Map::CalcCellPathFromParent(std::unordered_map<Pos, Pos> parent, Pos dest)
{
	// A* 개선
	std::vector<Vector2Int> cells;

	if (parent.find(dest) == parent.end())
	{
		Pos best;
		int32_t bestDist = INT32_MAX;

		for (auto value : parent)
		{
			auto pos = value.first;
			int32_t dist = std::abs(dest.X - pos.X) + std::abs(dest.Y - pos.Y);
			// 제일 우수한 후보를 뽑는다
			if (dist < bestDist)
			{
				best = pos;
				bestDist = dist;
			}
		}

		dest = best;
	}

	Pos pos = dest;
	while (parent[pos] != pos)
	{
		cells.push_back(PosToCell(pos));
		pos = parent[pos];
	}
	cells.push_back(PosToCell(pos));
	std::reverse(cells.begin(), cells.end());

	return cells;
}