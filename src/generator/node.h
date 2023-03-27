#pragma once


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


