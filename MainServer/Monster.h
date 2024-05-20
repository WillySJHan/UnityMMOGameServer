#pragma once
#include "BaseObject.h"
#include "Data.h"

class Player;

class Monster : public BaseObject
{
public:
	Monster();
	virtual ~Monster();
	void Init(int32_t templateId);

	virtual void Update() override;
	virtual void OnDead(std::shared_ptr<BaseObject> attacker) override;
	

protected:
	virtual void UpdateIdle();
	virtual void UpdateMoving();
	void BroadcastMove(std::shared_ptr<Room>& room);
	virtual void UpdateSkill();
	virtual void UpdateDead();

private:
	std::optional<RewardData> GetRandomReward();


public:
	// TODO : Atomic?
	std::weak_ptr<Player> _target;
	//std::shared_ptr<Task> _task;
	int32_t _searchCellDist = 10;
	int32_t _templateId = 0;

private:
	uint64_t _nextSearchTick = 0;
	uint64_t _nextMoveTick = 0;
	int32_t _chaseCellDist = 20;
	int32_t _skillRange = 1;
	uint64_t _coolTick = 0;
	std::shared_ptr<Task> _task;
};

