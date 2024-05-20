#include "pch.h"
#include "BaseObject.h"
#include "Room.h"

BaseObject::BaseObject(Protocol::ObjectType objectType) :_objectType(objectType), _cellPos(Vector2Int(0, 0))
{
    _objectInfo = new Protocol::ObjectInfo();
    _posInfo = new Protocol::PositionInfo();
    _statInfo = new Protocol::StatInfo();
    _objectInfo->set_allocated_pos_info(_posInfo);
    _objectInfo->set_allocated_stat_info(_statInfo);
}

BaseObject::~BaseObject()
{
	delete _objectInfo;
    std::cout << "~BaseObject" << std::endl;
}

void BaseObject::Update()
{

}

Vector2Int BaseObject::GetCellPos()
{
	_cellPos.x = _posInfo->pos_x();
	_cellPos.y = _posInfo->pos_y();

	return _cellPos;
}

void BaseObject::SetCellpos(Vector2Int pos)
{
	_posInfo->set_pos_x(pos.x);
	_posInfo->set_pos_y(pos.y);
}

Vector2Int BaseObject::GetFrontCellPos(Protocol::MoveDir dir)
{
    Vector2Int cellPos = GetCellPos();

    switch (dir) {
    case Protocol::MoveDir::UP:
        cellPos = cellPos + Vector2Int::up();
        break;
    case Protocol::MoveDir::DOWN:
        cellPos = cellPos + Vector2Int::down();
        break;
    case Protocol::MoveDir::LEFT:
        cellPos = cellPos + Vector2Int::left();
        break;
    case Protocol::MoveDir::RIGHT:
        cellPos = cellPos + Vector2Int::right();
        break;
    }

    return cellPos;
}

Vector2Int BaseObject::GetFrontCellPos()
{
    return GetFrontCellPos(_posInfo->move_dir());
}

Protocol::MoveDir BaseObject::GetDirFromVec(Vector2Int dir)
{
    if (dir.x > 0)
        return Protocol::MoveDir::RIGHT;
    else if (dir.x < 0)
        return Protocol::MoveDir::LEFT;
    else if (dir.y > 0)
        return Protocol::MoveDir::UP;
    else
        return Protocol::MoveDir::DOWN;
}

void BaseObject::OnDamaged(std::shared_ptr<BaseObject> attacker, int32_t damage)
{
    std::shared_ptr<Room> room = _room.load().lock();
    if (room == nullptr)
        return;

    damage = max(damage - TotalDeffence(), 0);
    _statInfo->set_hp(max(_statInfo->hp() - damage, 0));

    Protocol::MS_CHANGE_HP changeProto;
    changeProto.set_object_id(_objectInfo->object_id());
    changeProto.set_hp(_statInfo->hp());

    auto sendBuffer = ClientProtocolManager::MakeSendBuffer(changeProto);


    room->Broadcast(GetCellPos(), sendBuffer);
    

    if (_statInfo->hp() <= 0)
    {
        OnDead(attacker);
    }
}

void BaseObject::OnDead(std::shared_ptr<BaseObject> attacker)
{
    auto room = _room.load().lock();
    if (room == nullptr)
        return;

    Protocol::MS_DIE dieProto;
    dieProto.set_object_id(_objectInfo->object_id());
    dieProto.set_attacker_id(attacker->_objectInfo->object_id());

    auto SendBuffer = ClientProtocolManager::MakeSendBuffer(dieProto);

    room->Broadcast(GetCellPos(), SendBuffer);

    room->LeaveGame(shared_from_this());

    _statInfo->set_hp(_statInfo->max_hp());
    _posInfo->set_state(Protocol::CreatureState::IDLE);
    _posInfo->set_move_dir(Protocol::MoveDir::DOWN);

    room->EnterGame(shared_from_this(), true);
}

std::shared_ptr<BaseObject> BaseObject::GetOwner()
{
    return shared_from_this();
}

