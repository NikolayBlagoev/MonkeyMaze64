#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#include "node.h"
#include <deque>

class Generator {
    public:
        Generator() = default;
        
        void visualise(Defined*** board, int y, int x);
        void connect(Defined* a, Defined* b, int dir);
        int remove_options(Defined* node, int mm, int mx);
        int remove_own_options(Defined* node);
        void constrain(Defined* nd, int opts);
        void remove_head(int y, int x);
        void move_l(Defined*** board, std::deque <Defined*> *dq);
        void move_d(Defined*** board, std::deque <Defined*> *dq);
        void move_r(Defined*** board, std::deque <Defined*> *dq);
        void move_u(Defined*** board, std::deque <Defined*> *dq);

        void assign_all(std::deque <Defined*> *dq);
        void instantiate_terr();

    public:
        Defined*** board;
        std::deque <Defined*> dq;
        float acc_cam = 1.f;
        float acc_head = 1.f;
        float acc_vase = 1.f;
        float acc_box = 1.f;
        int heads = 0;
};

#endif
