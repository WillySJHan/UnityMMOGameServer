#include "pch.h"
#include "Bullet.h"
#include "Room.h"
#include "Map.h"

Bullet::~Bullet()
{
	std::cout << "~Bullet" << std::endl;
}

void Bullet::Update()
{
	if (_data == nullptr || _owner == nullptr)
		return;

	auto room = _room.load().lock();
	if (room == nullptr)
		return;

	uint64_t tick = static_cast<uint64_t>(1000 / _data->projectile.speed);
	auto bullet = shared_from_this();
	room->ReservePush(tick, [bullet]()
	{
			bullet->Update();
	});

	Vector2Int destPos = GetFrontCellPos();

	if (room->_map->ApplyMove(shared_from_this(), destPos, true, false))
	{
		Protocol::MS_MOVE moveProto;

		moveProto.set_object_id(_objectInfo->object_id());
		moveProto.mutable_pos_info()->CopyFrom(*_posInfo);
		auto sendBuffer = ClientProtocolManager::MakeSendBuffer(moveProto);

		room->Broadcast(GetCellPos(), sendBuffer);
	}
	else
	{
		std::shared_ptr<BaseObject> target = room->_map->Find(destPos);
		if (target != nullptr)
		{
			target->OnDamaged(shared_from_this(), _data->damage + _owner->TotalAttack());
		}
		// ¼Ò¸ê
		room->LeaveGame(shared_from_this());
	}
	

}

std::shared_ptr<BaseObject> Bullet::GetOwner()
{
	return _owner;
}
