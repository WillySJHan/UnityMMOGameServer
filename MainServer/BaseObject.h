#pragma once
#include "Map.h"
#include "Protocol.pb.h"

class Room;


class BaseObject : public std::enable_shared_from_this<BaseObject>
{
public:
	BaseObject() = delete;

	BaseObject(Protocol::ObjectType objectType);
	virtual ~BaseObject();

	virtual void Update();

	Vector2Int GetCellPos();
	void SetCellpos(Vector2Int pos);

	Vector2Int GetFrontCellPos(Protocol::MoveDir dir);
	Vector2Int GetFrontCellPos();

	static Protocol::MoveDir GetDirFromVec(Vector2Int dir);

	virtual void OnDamaged(std::shared_ptr<BaseObject> attacker, int32_t damage);
	virtual void OnDead(std::shared_ptr<BaseObject> attacker);

	virtual std::shared_ptr<BaseObject> GetOwner();

	virtual int32_t TotalAttack() { return _statInfo->attack(); };
	virtual int32_t TotalDeffence() { return 0; };

public:
	Protocol::ObjectType _objectType; //= Protocol::ObjectType::NONE;
	std::atomic<std::weak_ptr<Room>> _room;
	Protocol::ObjectInfo* _objectInfo;
	Protocol::PositionInfo* _posInfo;
	Protocol::StatInfo* _statInfo;

protected:
	Vector2Int _cellPos;


	// TODO : hp GET SET clamp?
};

