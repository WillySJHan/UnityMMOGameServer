#pragma once
#include "BaseObject.h"
#include "Player.h"

class ObjectManager
{
public:
    template <typename T>
    std::shared_ptr<T> Add();

    int32_t GenerateId(Protocol::ObjectType objectType);

    static Protocol::ObjectType GetObjectTypeById(int32_t objectId)
    {
        int32_t type = (objectId >> 24) & 0x7F;
        return static_cast<Protocol::ObjectType>(type);
    }

    bool Remove(int32_t objectId);

    std::shared_ptr<Player> Find(int32_t objectId);
    


private:
    std::recursive_mutex _rMutex;
    std::unordered_map<int32_t, std::shared_ptr<Player>> _players;
    //1 x ,7 type ,24 id
    int32_t _counter = 0; // TODO
};

template <typename T>
std::shared_ptr<T> ObjectManager::Add()
{
    std::shared_ptr<T> object = std::make_shared<T>();
    std::shared_ptr<BaseObject> baseObject = std::static_pointer_cast<BaseObject>(object);
    {
        std::lock_guard<std::recursive_mutex> lock(_rMutex);

        baseObject->_objectInfo->set_object_id(GenerateId(baseObject->_objectType));

        if (baseObject->_objectType == Protocol::ObjectType::PLAYER)
        {
            _players[baseObject->_objectInfo->object_id()] = std::static_pointer_cast<Player>(baseObject);
        }
    }
    object = std::static_pointer_cast<T>(baseObject);
    return object;
}

extern ObjectManager g_objectManager;

