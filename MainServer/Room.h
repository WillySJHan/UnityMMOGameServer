#pragma once

class Player;
class Map;
class Monster;
class Projectile;
class Area;

class Room : public TaskQueue
{
public:
	Room();

	void Init(int32_t mapId, int32_t areaCells);

	// In Game
	void EnterGame(std::shared_ptr<BaseObject> baseObject, bool randomPos);
	void LeaveGame(std::shared_ptr<BaseObject> baseObject);
	void ManageMove(std::shared_ptr<Player> player, Protocol::UC_MOVE moveProto);
	void ManageSkill(std::shared_ptr<Player> player, Protocol::UC_SKILL skillProto);
	void Broadcast(Vector2Int pos, std::shared_ptr<SendBuffer> sendBuffer);
	void ChangeRoom(std::shared_ptr<Player> player, Protocol::UC_CHANGE_ROOM changeRoomProto);

	// Item
	void ManageEquipItem(std::shared_ptr<Player> player, Protocol::UC_EQUIP_ITEM equipProto);

	// Area
	std::shared_ptr<Area> GetArea(Vector2Int cellPos);
	std::shared_ptr<Area> GetArea(int32_t indexY, int32_t indexX);
	std::shared_ptr<Player> FindNearPlayer(Vector2Int pos, int32_t searchRange);
	std::vector<std::shared_ptr<Player>> GetNearPlayers(Vector2Int pos, int32_t searchRange);
	std::vector<std::shared_ptr<Area>> GetNearAreas(Vector2Int cellPos, int32_t searchRange = Room::VisualFieldCells);




private:
	std::shared_ptr<Player> FindPlayer(std::function<bool(std::shared_ptr<BaseObject>)> condition);

private:
	bool EnterObject(std::shared_ptr<BaseObject> baseObject, bool randomPos);
	bool LeaveObject(int32_t objectId);

public:
	std::shared_ptr<Map> _map;
	int32_t _id = 0;

	std::vector<std::vector<std::shared_ptr<Area>>> _areas;
	int32_t _areaCells;
	static const int32_t VisualFieldCells = 5;

private:
	std::unordered_map<int32_t, std::shared_ptr<Player>> _players;
	std::unordered_map<int32_t, std::shared_ptr<Monster>> _monsters;
	std::unordered_map<int32_t, std::shared_ptr<Projectile>> _projectiles;
	
};

