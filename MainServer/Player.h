#pragma once
#include "BaseObject.h"
#include "Struct.pb.h"
#include "Map.h"

class Inventory;
class VisualField;
class ClientSession;

class Player : public BaseObject
{
public:
	Player();
	virtual ~Player();

	virtual void OnDamaged(std::shared_ptr<BaseObject> attacker, int32_t damage) override;
	virtual void OnDead(std::shared_ptr<BaseObject> attacker) override ;
	void OnLeaveGame();
	void ManageEquipItem(Protocol::UC_EQUIP_ITEM equipProto);
	void UpdateAdditionalStat();

	virtual int32_t TotalAttack() override { return _statInfo->attack() + _weaponDamage; };
	virtual int32_t TotalDeffence() override { return _armorDefence; };

public:
	std::weak_ptr<ClientSession> _session;
	int32_t _playerDbId = 0;
	std::shared_ptr<Inventory> _inven;
	std::shared_ptr<VisualField> _visualField;

	int32_t _weaponDamage = 0;
	int32_t _armorDefence = 0;

};
