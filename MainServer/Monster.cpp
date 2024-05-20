#include "pch.h"
#include "Monster.h"

#include "Data.h"
#include "DataManager.h"
#include "Room.h"
#include <optional>

#include "DBTransaction.h"

Monster::Monster() : BaseObject(Protocol::ObjectType::MONSTER)
{

}

Monster::~Monster()
{
}

void Monster::Init(int32_t templateId)
{
	_templateId = templateId;

	auto it = DataManager::MonsterMap.find(_templateId);
	if (it == DataManager::MonsterMap.end())
		return;

	_statInfo->CopyFrom(it->second.stat);
	_statInfo->set_hp(it->second.stat.max_hp());

	_posInfo->set_state(Protocol::CreatureState::IDLE);
}

void Monster::Update()
{
	// FSM
	// TODO 쿼드트리 개선
	switch (_posInfo->state())
	{
	case Protocol::CreatureState::IDLE:
		UpdateIdle();
		break;
	case Protocol::CreatureState::MOVING:
		UpdateMoving();
		break;
	case Protocol::CreatureState::SKILL:
		UpdateSkill();
		break;
	case Protocol::CreatureState::DEAD:
		UpdateDead();
		break;
	}

	 // 5프레임

	auto monster = shared_from_this();
	if (auto room = _room.load().lock())
	{
		_task = room->ReservePush(200,[monster]()
		{
				monster->Update();
		});
	}

}

void Monster::OnDead(std::shared_ptr<BaseObject> attacker)
{
	if (_task != nullptr)
	{
		_task->_cancel = true;
		_task = nullptr;
	}

	BaseObject::OnDead(attacker);

	std::shared_ptr<BaseObject> owner = attacker->GetOwner();

	if (owner->_objectType == Protocol::ObjectType::PLAYER)
	{
		auto rewardData = GetRandomReward();
		if (rewardData.has_value())
		{
			std::shared_ptr<Player> player = std::static_pointer_cast<Player>(owner);

			auto room = _room.load().lock();
			g_dbTransaction->RewardPlayer_Set(player, rewardData.value(), room);
		}
	}

}

void Monster::UpdateIdle()
{
	if (_nextSearchTick > GetTickCount64())
		return;

	_nextSearchTick = GetTickCount64() + 1000;

	auto room = _room.load().lock();
	if (room == nullptr)
		return;

	auto monster = std::static_pointer_cast<Monster>(shared_from_this());

	std::shared_ptr<Player> target = room->FindNearPlayer(GetCellPos(),_searchCellDist);
	if (target == nullptr)
		return;

	_target = target;

	_posInfo->set_state(Protocol::CreatureState::MOVING);
}

void Monster::UpdateMoving()
{
	if (_nextMoveTick > GetTickCount64())
		return;

	int32_t moveTick = static_cast<int32_t>(1000 / _statInfo->speed());
	_nextMoveTick = GetTickCount64() + moveTick;

	auto room = _room.load().lock();

	auto target = _target.lock();

	if (room == nullptr || target == nullptr || g_objectManager.Find(target->_objectInfo->object_id()) == nullptr)
	{
		_target = std::weak_ptr<Player>();
		_posInfo->set_state(Protocol::CreatureState::IDLE);
		BroadcastMove(room);
		return;
	}

	auto targetRoom = target->_room.load().lock();

	if (targetRoom == nullptr || targetRoom != room)
	{
		_target = std::weak_ptr<Player>();
		_posInfo->set_state(Protocol::CreatureState::IDLE);
		BroadcastMove(room);
		return;
	}

	Vector2Int dir = target->GetCellPos() - GetCellPos();
	int32_t dist = dir.GetCellDistFromZero();


	if (dist == 0 || dist > _chaseCellDist)
	{
		_target = std::weak_ptr<Player>();
		_posInfo->set_state(Protocol::CreatureState::IDLE);
		BroadcastMove(room);
		return;
	} 

	std::vector<Vector2Int> path = room->_map->FindPath(GetCellPos(), target->GetCellPos(), true);
	if (path.size() < 2 || path.size() > _chaseCellDist)
	{
		_target = std::weak_ptr<Player>();
		_posInfo->set_state(Protocol::CreatureState::IDLE);
		BroadcastMove(room);
		return;
	}

	//스킬쓸까?
	if (dist <= _skillRange && (dir.x == 0 || dir.y == 0)) // 대각선x
	{
		_coolTick = 0;
		_posInfo->set_state(Protocol::CreatureState::SKILL);
		return;
	}

	// 이동
	_posInfo->set_move_dir(GetDirFromVec(path[1] - GetCellPos()));
	room->_map->ApplyMove(shared_from_this(), path[1]);
	BroadcastMove(room);
}

void Monster::BroadcastMove(std::shared_ptr<Room>& room)
{
	Protocol::MS_MOVE moveProto;

	moveProto.set_object_id(_objectInfo->object_id());
	moveProto.mutable_pos_info()->CopyFrom(*_posInfo);

	auto SendBuffer = ClientProtocolManager::MakeSendBuffer(moveProto);
	room->Broadcast(GetCellPos(), SendBuffer);
}

void Monster::UpdateSkill()
{
	if (_coolTick == 0)
	{
		auto target = _target.lock();

		auto room = _room.load().lock();


		// valid target?
		if (target == nullptr || room == nullptr || target->_statInfo->hp() == 0 || g_objectManager.Find(target->_objectInfo->object_id()) == nullptr )
		{
			_target = std::weak_ptr<Player>();
			_posInfo->set_state(Protocol::CreatureState::MOVING);
			BroadcastMove(room);
			return;
		}

		auto targetRoom = target->_room.load().lock();

		if (targetRoom == nullptr || targetRoom != room)
		{
			_target = std::weak_ptr<Player>();
			_posInfo->set_state(Protocol::CreatureState::MOVING);
			BroadcastMove(room);
			return;
		}

		//can skill?
		Vector2Int dir = (target->GetCellPos() - GetCellPos());
		int32_t dist = dir.GetCellDistFromZero();
		bool canUseSkill = (dist <= _skillRange && (dir.x == 0 || dir.y == 0));
		if (canUseSkill == false)
		{
			//_target = nullptr;
			_posInfo->set_state(Protocol::CreatureState::MOVING);
			BroadcastMove(room);
			return;
		}

		//타겟팅 방향 주시
		Protocol::MoveDir lookDir = GetDirFromVec(dir);
		if (_posInfo->move_dir() != lookDir)
		{
			_posInfo->set_move_dir(lookDir);
			BroadcastMove(room);
		}

		Skill skillData = DataManager::SkillMap[1];
		// TODO : skill data 있을까?

		//데미지 판정
		target->OnDamaged(shared_from_this(), skillData.damage + TotalAttack());

		// 스킬 사용 Broadcast
		Protocol::MS_SKILL skill;
		skill.set_object_id(_objectInfo->object_id());
		skill.mutable_info()->set_skill_id(skillData.id);
		auto SendBuffer = ClientProtocolManager::MakeSendBuffer(skill);

		room->Broadcast(GetCellPos(), SendBuffer);

		//스킬 쿨타임 적용
		int32_t coolTick = static_cast<int32_t>(1000 * skillData.cooldown);
		_coolTick = GetTickCount64() + coolTick;
	}

	if (_coolTick > GetTickCount64())
		return;

	_coolTick = 0;
}

void Monster::UpdateDead()
{
}

std::optional<RewardData> Monster::GetRandomReward()
{
	MonsterData monsterData = DataManager::MonsterMap.find(_templateId)->second;

	int32_t rand = Utility::GetRandom(0, 100);

	int32_t sum = 0;
	for(auto rewardData : monsterData.rewards)
	{
		sum += rewardData.probability;
		if (rand <= sum)
			return rewardData;
	}
	
	return std::nullopt;
}
