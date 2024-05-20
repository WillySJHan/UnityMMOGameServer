#pragma once
#include "Protocol.pb.h"


class ProjectileInfo
{
public:
	std::string name;
	float speed;
	int32_t range;
	std::string prefab;
};

class Skill
{
public:
	int32_t id;
	std::string name;
	float cooldown;
	int32_t damage;
	Protocol::SkillType skillType;
	ProjectileInfo projectile;
};

class ItemData
{
public:
	int32_t id;
	std::string name;
	Protocol::ItemType itemType;
};

class WeaponData : public ItemData
{
public:
	Protocol::WeaponType weaponType;
	int32_t damage;
};

class ArmorData : public ItemData
{
public:
	Protocol::ArmorType armorType;
	int32_t defence;
};

class ConsumableData : public ItemData
{
public:
	Protocol::ConsumableType consumableType;
	int32_t maxCount;
};


class RewardData;

class MonsterData
{
public:
	int32_t id;
	std::string name;
	Protocol::StatInfo stat;
	std::vector<RewardData> rewards;
	// std::string prefabPath;
};

class RewardData
{
public:
	int32_t probability;
	int32_t itemId;
	int32_t count;
};