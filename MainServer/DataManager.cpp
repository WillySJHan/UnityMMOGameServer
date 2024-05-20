#include "pch.h"
#include "DataManager.h"
#include "Data.h"

std::unordered_map<int32_t, Protocol::StatInfo> DataManager::StatMap;
std::unordered_map<int32_t, Skill> DataManager::SkillMap;
std::unordered_map<int32_t, ItemData*> DataManager::ItemMap;
std::unordered_map<int32_t, MonsterData> DataManager::MonsterMap;

void DataManager::LoadData()
{
    SkillMap = LoadJson<SkillData, int32_t, Skill>("SkillData").MakeMap();
    StatMap = LoadJson<StatData, int32_t, Protocol::StatInfo>("StatData").MakeMap();
    ItemMap = LoadJson<ItemLoader, int32_t, ItemData*>("ItemData").MakeMap();
    MonsterMap = LoadJson<MonsterLoader, int, MonsterData>("MonsterData").MakeMap();

}

void StatData::setData(const nlohmann::json& jsonData)
{
    if (jsonData.find("stats") != jsonData.end() && jsonData["stats"].is_array())
    {
        for (const auto& stat : jsonData["stats"])
        {
            Protocol::StatInfo s;
            s.set_level(std::stoi(stat["level"].get<std::string>()));
            s.set_max_hp(std::stoi(stat["maxHp"].get<std::string>()));
            s.set_attack(std::stoi(stat["attack"].get<std::string>()));
            s.set_speed(std::stof(stat["speed"].get<std::string>()));
            s.set_total_exp(std::stoi(stat["totalExp"].get<std::string>()));
            s.set_hp(s.max_hp());
            _stats.push_back(s);
        }
    }
}

void SkillData::setData(const nlohmann::json& jsonData)
{
    for (const auto& skillJson : jsonData["skills"])
    {
        Skill skill;
        skill.id = std::stoi(skillJson["id"].get<std::string>());
        skill.name = skillJson["name"].get<std::string>();

        skill.cooldown = std::stof(skillJson["cooldown"].get<std::string>());
        skill.damage = std::stoi(skillJson["damage"].get<std::string>());

        if (skillJson["skillType"] == "SkillAuto")
        {
            skill.skillType = Protocol::SkillType::SKILL_AUTO;
        }
        else if (skillJson["skillType"] == "SkillProjectile")
        {
            skill.skillType = Protocol::SkillType::SKILL_PROJECTILE;
            skill.projectile.name = skillJson["projectile"]["name"].get<std::string>();
            skill.projectile.speed = std::stof(skillJson["projectile"]["speed"].get<std::string>());
            skill.projectile.range = std::stoi(skillJson["projectile"]["range"].get<std::string>());
            skill.projectile.prefab = skillJson["projectile"]["prefab"].get<std::string>();
        }

        _skills.push_back(skill);
    }
}

void ItemLoader::setData(const nlohmann::json& jsonData)
{
    for (const auto& weaponJson : jsonData["weapons"])
    {
        WeaponData* weapon = new WeaponData;
        weapon->id = std::stoi(weaponJson["id"].get<std::string>());
        weapon->name = weaponJson["name"].get<std::string>();

        if (weaponJson["weaponType"] == "Sword")
            weapon->weaponType = Protocol::WEAPON_TYPE_SWORD;
        else if (weaponJson["weaponType"] == "Bow")
            weapon->weaponType = Protocol::WEAPON_TYPE_BOW;

        weapon->damage = std::stoi(weaponJson["damage"].get<std::string>());
        _weapons.push_back(weapon);


        
    }

    for (const auto& armorJson : jsonData["armors"])
    {
        ArmorData* armor = new ArmorData;
        armor->id = std::stoi(armorJson["id"].get<std::string>());
        armor->name = armorJson["name"].get<std::string>();

        if (armorJson["armorType"] == "Armor")
            armor->armorType = Protocol::ARMOR_TYPE_ARMOR;
        else if (armorJson["armorType"] == "Helmet")
            armor->armorType = Protocol::ARMOR_TYPE_HELMET;
        else if (armorJson["armorType"] == "Boots")
            armor->armorType = Protocol::ARMOR_TYPE_BOOTS;

        armor->defence = std::stoi(armorJson["defence"].get<std::string>());
        _armors.push_back(armor);
    }

    for (const auto& consumableJson : jsonData["consumables"])
    {
        ConsumableData* consumable = new ConsumableData;
        consumable->id = std::stoi(consumableJson["id"].get<std::string>());
        consumable->name = consumableJson["name"].get<std::string>();

        if (consumableJson["consumableType"] == "Potion")
            consumable->consumableType = Protocol::CONSUMABLE_TYPE_POTION;

        consumable->maxCount = std::stoi(consumableJson["maxCount"].get<std::string>());
        _consumables.push_back(consumable);
    }
}

void MonsterLoader::setData(const nlohmann::json& jsonData)
{
    for (const auto& monsterJson : jsonData["monsters"])
    {
        MonsterData monster;
        monster.id = std::stoi(monsterJson["id"].get<std::string>());
        monster.name = monsterJson["name"].get<std::string>();

        monster.stat.set_level(std::stoi(monsterJson["stat"]["level"].get<std::string>()));
        monster.stat.set_max_hp(std::stoi(monsterJson["stat"]["maxHp"].get<std::string>()));
        monster.stat.set_attack(std::stoi(monsterJson["stat"]["attack"].get<std::string>()));
        monster.stat.set_speed(std::stof(monsterJson["stat"]["speed"].get<std::string>()));
        monster.stat.set_total_exp(std::stoi(monsterJson["stat"]["totalExp"].get<std::string>()));

        
        for (const auto& rewardJson : monsterJson["rewards"])
        {
            RewardData reward;
            reward.probability = std::stoi(rewardJson["probability"].get<std::string>());
            reward.itemId = std::stoi(rewardJson["itemId"].get<std::string>());
            reward.count = std::stoi(rewardJson["count"].get<std::string>());
            monster.rewards.push_back(reward);
        }
        _monsters.push_back(monster);
    }
}
