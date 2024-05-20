#pragma once

class Player;
class Monster;
class Projectile;


class Area
{
public:
	Area() = default;
	Area(int32_t y, int32_t x);

	std::shared_ptr<Player> FindOnePlayer(std::function<bool(std::shared_ptr<Player>)> condition);
	std::vector<std::shared_ptr<Player>> FindAllPlayers(std::function<bool(std::shared_ptr<Player>)> condition);
	void  Remove(std::shared_ptr<BaseObject> baseObject);
public:
	int32_t _indexY = 0;
	int32_t _indexX = 0;
	std::unordered_set<std::shared_ptr<Player>> _players;
	std::unordered_set<std::shared_ptr<Monster>> _monsters;
	std::unordered_set<std::shared_ptr<Projectile>> _projectiles;
};
