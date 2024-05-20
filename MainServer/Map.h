#pragma once
#include <cmath>
#include <fstream>
#include <sstream>

class Player;
class BaseObject;

struct Pos
{
    Pos() : Y(0), X(0) {}
    Pos(int32_t y, int32_t x) : Y(y), X(x) {}
    int32_t Y;
    int32_t X;

    bool operator==(const Pos& rhs) const
    {
        return Y == rhs.Y && X == rhs.X;
    }

    bool operator!=(const Pos& rhs) const
    {
        return !(*this == rhs);
    }
};

namespace std
{
    template <>
    struct hash<Pos>
    {
        size_t operator()(const Pos& p) const
        {
            //return hash<int32_t>()(p.Y) ^ hash<int32_t>()(p.X);
            std::hash<uint64_t> hasher;
            uint64_t value = (static_cast<uint64_t>(p.Y) << 32) | static_cast<uint64_t>(p.X);
            return hasher(value);
        }
    };
}


struct PQNode
{
    int32_t F;
    int32_t G;
    int32_t Y;
    int32_t X;

    bool operator<(const PQNode& other) const { return F < other.F; }
    bool operator>(const PQNode& other) const { return F > other.F; }
};

struct Vector2Int
{
    int32_t x;
    int32_t y;

    Vector2Int(int x, int y) : x(x), y(y) {}

    static Vector2Int up() { return Vector2Int(0, 1); }
    static Vector2Int down() { return Vector2Int(0, -1); }
    static Vector2Int left() { return Vector2Int(-1, 0); }
    static Vector2Int right() { return Vector2Int(1, 0); }

    Vector2Int operator+(const Vector2Int& a)
    {
        return Vector2Int(x + a.x, y + a.y);
    }

    Vector2Int operator-(const Vector2Int& a)
    {
        return Vector2Int(x - a.x, y - a.y);
    }

    Vector2Int operator/(const int32_t a)
    {
        return Vector2Int(x / a, y / a);
    }

    int32_t GetSqrMagnitude() { return (x * x + y * y); }
    float GetMagnitude() { return static_cast<float>(std::sqrt(GetSqrMagnitude())); }
    int32_t GetCellDistFromZero() { return (std::abs(x) + std::abs(y)); }
};

class Map : public std::enable_shared_from_this<Map>
{
public:
    bool CanGo(Vector2Int cellPos, bool checkObjects = true);

    void LoadMap(int32_t mapId, const std::string& pathPrefix = "../Resource/Map");
    std::shared_ptr<BaseObject> Find(Vector2Int cellPos);
    bool ApplyMove(std::shared_ptr<BaseObject> baseObject, Vector2Int dest, bool checkObjects = true, bool collision = true);
    bool ApplyLeave(std::shared_ptr<BaseObject> baseObject);

   

    Pos CellToPos(Vector2Int cell)
    {
        return Pos(_maxY - cell.y, cell.x - _minX);
    }

    Vector2Int PosToCell(Pos pos)
    {
        return Vector2Int(pos.X + _minX, _maxY - pos.Y);
    }

    //A*

    std::vector<Vector2Int> FindPath(Vector2Int startCellPos, Vector2Int destCellPos, bool checkObjects = true, int32_t maxDist = 10);
    std::vector<Vector2Int> CalcCellPathFromParent(std::unordered_map<Pos, Pos> parent, Pos dest);
public:
    std::vector<std::vector<int32_t>> _collision;
    std::vector<std::vector<std::shared_ptr<BaseObject>>> _objects;


    int32_t _minX = 0;
    int32_t _minY = 0;
    int32_t _maxX = 0;
    int32_t _maxY = 0;

    int32_t SizeX() { return _maxX - _minX + 1; }
    int32_t SizeY() { return _maxY - _minY + 1; }


private:
    std::vector<int32_t> _deltaY{ 1, -1, 0, 0 };
    std::vector<int32_t> _deltaX{ 0, 0, -1, 1 };
    std::vector<int32_t> _cost{ 10, 10, 10, 10 };
};

