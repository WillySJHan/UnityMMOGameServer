#pragma once
#include "json.hpp"
#include "ConfigManager.h"
#include <fstream>
#include "Data.h"

template <typename T, typename U>
class ILoader {
public:
    virtual std::unordered_map<T, U> MakeMap() = 0;
    virtual void setData(const nlohmann::json& jsonData) = 0;
};

struct Stat
{
    int32_t level;
    int32_t maxHp;
    int32_t attack;
    int32_t tatalExp;
};
class Skill;
class ItemData;


class DataManager
{
public:
    static std::unordered_map<int32_t, Protocol::StatInfo> StatMap;
    static std::unordered_map<int32_t, Skill> SkillMap;
    static std::unordered_map<int32_t, ItemData*> ItemMap;
    static std::unordered_map<int32_t, MonsterData> MonsterMap;

    static void LoadData();

    template <typename Loader, typename T, typename U>
    static Loader LoadJson(const std::string& path) {
        std::string a = ConfigManager::Config.dataPath + "/" + path + ".json";
        std::ifstream file(a);
        nlohmann::json jsonData;
        Loader loader;
        if (file.is_open())
        {
        	file >> jsonData;

            loader.setData(jsonData);
        }
        file.close();
        return loader;
    }
};

class StatData :public ILoader<int32_t, Protocol::StatInfo>
{
public:
    virtual std::unordered_map<int32_t, Protocol::StatInfo> MakeMap()
    {
        std::unordered_map<int32_t, Protocol::StatInfo> map;
        for (Protocol::StatInfo stat : _stats) {
            stat.set_hp(stat.max_hp());
            //map.emplace(stat.level(), stat);
            map[stat.level()] = stat;
        }
        return map;
    }

    virtual void setData(const nlohmann::json& jsonData);

public:
    std::vector<Protocol::StatInfo> _stats;

};

class SkillData : public ILoader<int, Skill>
{
public:
    std::unordered_map<int, Skill> MakeMap()
    {
        std::unordered_map<int, Skill> map;
        for (Skill skill : _skills) {
            //map.emplace(skill.id, skill);
            map[skill.id] = skill;
        }
        return map;
    }

    virtual void setData(const nlohmann::json& jsonData);
public:
    std::vector<Skill> _skills;
};

class ItemLoader : public ILoader<int32_t, ItemData*>
{
public:
    std::unordered_map<int32_t, ItemData*> MakeMap()
    {
        std::unordered_map<int, ItemData*> map;
        for (auto item : _weapons) {
            item->itemType = Protocol::ITEM_TYPE_WEAPON;
            map[item->id] = item;

        }
        for (auto item : _armors)
        {
            item->itemType = Protocol::ITEM_TYPE_ARMOR;
            map[item->id] = item;
        }
        for (auto item : _consumables)
        {
            item->itemType = Protocol::ITEM_TYPE_CONSUMABLE;
            map[item->id] = item;
        }
        return map;
    }

    virtual void setData(const nlohmann::json& jsonData);
public:
    std::vector<WeaponData*> _weapons;
    std::vector<ArmorData*> _armors;
    std::vector<ConsumableData*> _consumables;
};

class MonsterLoader : public ILoader<int32_t, MonsterData>
{
public:
    std::unordered_map<int32_t, MonsterData> MakeMap()
    {
        std::unordered_map<int32_t, MonsterData> map;
        for (MonsterData monster : _monsters) 
        {
            map[monster.id] = monster;
        }
        return map;
    }

    virtual void setData(const nlohmann::json& jsonData);
public:
    std::vector<MonsterData> _monsters;

};