#pragma once


#include <glm/vec3.hpp>


#include <vector>
class ProcObj{
    public:
        ProcObj(int tt,  glm::vec3 scale, glm::vec3 selfRotate, glm::vec3 rotateParent, glm::vec3 translate) : type(tt), scale(scale), 
        selfRotate(selfRotate), rotateParent(rotateParent), translate(translate){

        };
        ProcObj(int tt) : type(tt){
            
        }
        int type = 0;
        glm::vec3 scale;
        glm::vec3 selfRotate; // Angles in degrees
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
        bool constrained = false;
        bool empt = true;
        bool possible[18] = {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true};
        std::vector<ProcObj*> objs;
        Defined(Defined* up, Defined* down, Defined* left, Defined* right, int tt) : up(up), down(down), left(left), right(right), tileType(tt){
            // up = upn;
            // this->down = down;
            // this->left = left;
            // this->up = up;
            // this->tileType = tt;
           
            this->empt = false;
        }
        Defined(int tt) : up(nullptr), down(nullptr), left(nullptr), right(nullptr), tileType(tt){
            this->empt = false;
        }
        Defined() : up(nullptr), down(nullptr), left(nullptr), right(nullptr), tileType(-1){
            // up = upn;
            // this->down = down;
            // this->left = left;
            // this->up = up;
            // this->tileType = tt;
            
            this->empt = true;
        }

        
};


