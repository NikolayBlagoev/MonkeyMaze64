#ifndef _NODE_H_
#define _NODE_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

#include <array>
#include <iostream>
#include <vector>

enum class SpecialObjType { Collectible, EnemyCamera };
enum class TileType { INVALID,
                      CROSSING,                     // Values as of this one MUST start at 1 for pre-existing generation arithmetic to work. I tried
                      ROOM1, ROOM2, ROOM3, ROOM4,   // My assumption as a dude doing refactoring is that these correspond to left-right-up-down, but I don't know which is which
                      EMPTY,
                      TUNNEL1, TUNNEL2, TUNNEL3, TUNNEL4,
                      TURN1, TURN2, TURN3, TURN4,
                      TJUNCTION1, TJUNCTION2, TJUNCTION3, TJUNCTION4 };
static std::ostream& operator << (std::ostream& os, const TileType& tile) {
   os << static_cast<std::underlying_type<TileType>::type>(tile);
   return os;
}

class ProcObj {
    public:
        ProcObj(SpecialObjType tt, glm::vec3 scale, glm::vec3 selfRotate, glm::vec3 rotateParent, glm::vec3 translate)
        : type(tt)
        , scale(scale)
        , selfRotate(selfRotate)
        , rotateParent(rotateParent)
        , translate(translate)
        {};
        ProcObj(SpecialObjType tt) : type(tt)
        {};

        SpecialObjType type { SpecialObjType::Collectible };
        glm::vec3 scale;
        glm::vec3 selfRotate;   // Angles in degrees
        glm::vec3 rotateParent; // Angles in degrees
        glm::vec3 translate;
};

class Defined {
    public:
        Defined* up;
        Defined* down;
        Defined* right;
        Defined* left;

        TileType tileType;
        std::vector<ProcObj*> objs;
        bool constrained                = false;
        bool empty                      = true;
        std::array<bool, 18UL> possible = {true, true, true, true, true, true,
                                           true, true, true, true, true, true,
                                           true, true, true, true, true, true};

        Defined(Defined* up, Defined* down, Defined* left, Defined* right, TileType tt, bool empty = false)
        : up(up)
        , down(down)
        , left(left)
        , right(right)
        , tileType(tt)
        , empty(empty)
        {}
        Defined(TileType tt)    : Defined(nullptr, nullptr, nullptr, nullptr, tt) {}
        Defined()               : Defined(nullptr, nullptr, nullptr, nullptr, TileType::INVALID, true) {}
};
#endif
