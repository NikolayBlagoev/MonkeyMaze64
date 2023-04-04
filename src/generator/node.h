#ifndef _NODE_H_
#define _NODE_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

#include <array>
#include <vector>

enum class SpecialObjType { Collectible, EnemyCamera };

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

        int tileType;
        std::vector<ProcObj*> objs;
        bool constrained                = false;
        bool empty                      = true;
        std::array<bool, 18UL> possible = {true, true, true, true, true, true,
                                           true, true, true, true, true, true,
                                           true, true, true, true, true, true};

        Defined(Defined* up, Defined* down, Defined* left, Defined* right, int tt, bool empty = false)
        : up(up)
        , down(down)
        , left(left)
        , right(right)
        , tileType(tt)
        , empty(empty)
        {}
        Defined(int tt) : Defined(nullptr, nullptr, nullptr, nullptr, tt) {}
        Defined()       : Defined(nullptr, nullptr, nullptr, nullptr, -1, true) {}
};
#endif
