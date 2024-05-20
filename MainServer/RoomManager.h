#pragma once

class Map;

class RoomManager : public TaskQueue
{
public:
    RoomManager();

    std::shared_ptr<class Room> Add(int32_t mapId);

    bool Remove(int32_t roomId);

    std::shared_ptr<class Room> Find(int32_t roomId);

public:
    std::shared_ptr<Map> _map;

private:
    std::recursive_mutex _rMutex;
    std::unordered_map<int32_t, std::shared_ptr<class Room>> _rooms;
    int32_t _roomId = 1;
};

extern std::shared_ptr<RoomManager> g_roomManager;

