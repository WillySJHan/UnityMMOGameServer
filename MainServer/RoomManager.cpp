#include "pch.h"
#include "RoomManager.h"
#include "Room.h"
#include "Map.h"

std::shared_ptr<RoomManager> g_roomManager = std::make_shared<RoomManager>();

RoomManager::RoomManager()
{
	_map = std::make_shared<Map>();
}

std::shared_ptr<Room> RoomManager::Add(int32_t mapId)
{
	std::shared_ptr<Room> room = std::make_shared<Room>();
	

	std::lock_guard<std::recursive_mutex> lock(_rMutex);
	{
		room->_id = _roomId;
		_rooms[_roomId] = room;
		_roomId++;
	}

	room->_map = _map;
	room->PushTask((&Room::Init), mapId, 10);
	return room;
}

bool RoomManager::Remove(int32_t roomId)
{
	std::lock_guard<std::recursive_mutex> lock(_rMutex);
	return _rooms.erase(roomId);
}

std::shared_ptr<Room> RoomManager::Find(int32_t roomId)
{
	std::lock_guard<std::recursive_mutex> lock(_rMutex);

	auto it = _rooms.find(roomId);

	if (it != _rooms.end()) {
		return it->second;
	}

	return nullptr;
}
