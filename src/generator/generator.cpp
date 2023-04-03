#include "node.h"

#include <iostream>
#include <vector>
#include <time.h>
#include <cstdlib>
#include <cmath>
#include "generator.h"
const bool opens[18][4] = {
    //1:
    {
        //up:
        true,
        //right:
        true,
        //down:
        true,
        //left:
        true
        
        
    },
    //2:
     {
        //up:
        false,
        //right:
        false,
        //down:
        true,
        //left:
        false
    },
    //3:
     {
        //up:
        false,
        //right:
        false,
        //down:
        false,
        //left:
        true
    },
    //4:
    {
        //up:
        true,
        //right:
        false,
        //down:
        false,
        //left:
        false
    },
    //5:
     {
        //up:
        false,
        //right:
        true,
        //down:
        false,
        //left:
        false
    },
    //6:
     {
        //up:
        false,
        //right:
        false,
        //down:
        false,
        //left:
        false
    },
    //7:
     {
        //up:
        true,
        //right:
        false,
        //down:
        true,
        //left:
        false
    },
    //8:
     {
        //up:
        false,
        //right:
        true,
        //down:
        false,
        //left:
        true
    },
    //9:
     {
        //up:
        true,
        //right:
        false,
        //down:
        true,
        //left:
        false
    },
    //10:
     {
        //up:
        false,
        //right:
        true,
        //down:
        false,
        //left:
        true
    },
    //11:
     {
        //up:
        false,
        //right:
        true,
        //down:
        true,
        //left:
        false
    },
    //12:
     {
        //up:
        false,
        //right:
        false,
        //down:
        true,
        //left:
        true
    },
    //13:
     {
        //up:
        true,
        //right:
        false,
        //down:
        false,
        //left:
        true
    },
    //14:
     {
        //up:
        true,
        //right:
        true,
        //down:
        false,
        //left:
        false
    },
    //15:
     {
        //up:
        false,
        //right:
        true,
        //down:
        true,
        //left:
        true,
    },
    //16:
     {
        //up:
        true,
        //right:
        false,
        //down:
        true,
        //left:
        true
    },
    //17:
     {
        //up:
        true,
        //right:
        true,
        //down:
        false,
        //left:
        true
    },
    //18:
     {
        //up:
        true,
        //right:
        true,
        //down:
        true,
        //left:
        false
    }
}; 

void Generator::visualise(Defined*** board, int y, int x){
    for (int i = 0; i < y; i ++){
        for (int j = 0; j < x; j ++){
            std::cout<<board[i][j]->tileType<<"\t";    
        }
        std::cout<<std::endl;
    }
}
void Generator::connect(Defined* a, Defined* b, int dir){
    if(dir == 0){
        a->up = b;
        b->down = a;
    }else if(dir == 1){
        a->right = b;
        b->left = a;
    }else if(dir == 2){
        a->down = b;
        b->up = a;
    }else if(dir == 3){
        a->left = b;
        b->right = a;
    }
} 
int Generator::remove_options(Defined* node, int mm, int mx){
    if(node == nullptr || node->empt){
        return 1099;
    }
    int min_node = 99;
    int dir_min = 0;
    Defined* up = node->up;
    
    if(up != nullptr && up->empt){
        int count = 0;
        for(int i = 0; i < 18; i ++){
            up->possible[i] = up->possible[i] && opens[node->tileType-1][0] == opens[i][2];
            if(!up->possible[i]) count++;
        }
        if(count < min_node){
            min_node = count;
            dir_min = 0;
        }
    }

    Defined* right = node->right;
    if(right != nullptr && right->empt){
        int count = 0;
        for(int i = 0; i < 18; i ++){
            right->possible[i] = right->possible[i] && opens[node->tileType-1][1] == opens[i][3];
            if(!right->possible[i]) count++;
        }
        if(count < min_node){
            min_node = count;
            dir_min = 1;
        }
    }

    Defined* down = node->down;
    
    if(down != nullptr && down->empt){
        int count = 0;
        for(int i = 0; i < 18; i ++){
            down->possible[i] = down->possible[i] && opens[node->tileType-1][2] == opens[i][0];
            if(!down->possible[i]) count++;
        }
        if(count < min_node){
            min_node = count;
            dir_min = 2;
        }
    }


    Defined* left = node->left;
    if(left != nullptr && left->empt){
        int count = 0;
        for(int i = 0; i < 18; i ++){
            left->possible[i] = left->possible[i] && opens[node->tileType-1][3] == opens[i][1];
            if(!left->possible[i]) count++;
        }
        if(count < min_node){
            min_node = count;
            dir_min = 3;
        }
    }
    return dir_min*100 + min_node;
}

void Generator::constrain(Defined* nd, int opts){
    if(nd == nullptr || !nd->empt){
        return;
    }
    bool flag = true;
    while(flag){
        int chs = rand() % (18-opts);
        int j = 0;
        for(int i = 0; i < 18; i ++){
            if(nd->possible[i]){
                if(j == chs){
                    if(i!=5 || opts > 16){
                        flag = false;
                    }
                    if((i >= 1 && i <= 4) || i>=14){
                        float res = exp(-0.2f*(rand()%10));
                        if( true || res > acc_cam ){
                            std::cout<<"ADDING CAMERA "<<res << " "<<acc_cam<<std::endl;
                            nd->objs.push_back(new ProcObj(0));
                            acc_cam = 1.f;
                        }else{
                            std::cout<<"NOT ADDING CAMERA "<<res << " "<<acc_cam<<std::endl;
                            acc_cam -= 0.1f;
                        }
                    }


                    if((i >= 1 && i <= 4) || i == 0){
                        float res = exp(-0.2f*(rand()%20));
                        if( true || res > acc_head ){
                            std::cout<<"ADDING HEAD "<<res << " "<<acc_head<<std::endl;
                            nd->objs.push_back(new ProcObj(1));
                            acc_head = 1.f;
                        }else{
                            std::cout<<"NOT ADDING HEAD "<<res << " "<<acc_head<<std::endl;
                            acc_head -= 0.05f;
                        }
                    }
                    nd->empt = false;
                    nd->tileType = i+1;
                    
                    break;
                }
                j++;
            }
        }
    }
    int ret = remove_options(nd,0,0);
    Defined* max_node = nullptr;
    if(ret/100 == 0){
        max_node = nd->up;
                
    }else if(ret/100 == 1){
        max_node = nd->right;
    }else if(ret/100 == 2){
        max_node = nd->down;
    }else if(ret/100 == 3){
        max_node = nd->left;
    }else{
        return;
    }
    constrain(max_node,ret%100);
}


void Generator::move_l(Defined*** board, std::deque <Defined*> *dq){
    for(int i = 0; i < 7; i ++){
        for(int j = 6; j > 0 ; j--){
            board[i][j] = board[i][j-1];
        }
    }
    for(int i = 0; i < 7; i ++){
        board[i][0] = board[i][1]->left;
        if(board[i][0] == nullptr){
            board[i][0] = new Defined();
            connect(board[i][1], board[i][0], 3);
            dq->push_back(board[i][0]);
        }
    }
}

void Generator::move_d(Defined*** board, std::deque <Defined*> *dq){
    for(int j = 0; j < 7; j ++){
        for(int i = 0; i < 6; i++){
            board[i][j] = board[i+1][j];
        }
    }
    for(int i = 0; i < 7; i ++){
        board[6][i] = board[5][i]->down;
        if(board[6][i] == nullptr){
            board[6][i] = new Defined();
            connect(board[5][i], board[6][i], 2);
            dq->push_back(board[6][i]);
        }
    }
}


void Generator::move_r(Defined*** board, std::deque <Defined*> *dq){
    for(int i = 0; i < 7; i ++){
        for(int j = 0; j < 6; j++){
            board[i][j] = board[i][j+1];
        }
    }
    for(int i = 0; i < 7; i ++){
        board[i][6] = board[i][5]->right;
        if(board[i][6] == nullptr){
            board[i][6] = new Defined();
            connect(board[i][5], board[i][6], 1);
            dq->push_back(board[i][6]);
        }
    }
}

void Generator::move_u(Defined*** board, std::deque <Defined*> *dq){
    for(int j = 0; j < 7; j ++){
        for(int i = 6; i > 0 ; i--){
            board[i][j] = board[i-1][j];
        }
    }
    for(int i = 0; i < 7; i ++){
        board[0][i] = board[1][i]->up;
        if(board[0][i] == nullptr){
            board[0][i] = new Defined();
            connect(board[1][i], board[0][i], 0);
            dq->push_back(board[0][i]);
        }
    }
}
void Generator::assign_all(std::deque <Defined*> *dq){
    while(!dq->empty()){
        Defined* curr = dq->front();
        dq->pop_front();
        if(curr == nullptr) continue;
        if(!curr->empt) continue;
        int count = 0;
        for(int i = 0; i < 18; i ++){
            if(!curr->possible[i]){
                count ++;
            }
        }
        constrain(curr, count);
    }
}
void Generator::instantiate_terr(){
    
    srand(time(0));
    
    board = new Defined**[7];
    for(int i = 0; i < 7; i ++){
        board[i] = new Defined*[7];
    }
    // board[0][0] = 
    Defined* tempt[7][7] =   {  
        {new Defined(), new Defined(), new Defined(), new Defined(), new Defined(), new Defined(), new Defined()},
        {new Defined(), new Defined(), new Defined(), new Defined(10), new Defined(), new Defined(), new Defined()},
        {new Defined(), new Defined(), new Defined(), new Defined(11), new Defined(), new Defined(), new Defined()},
        {new Defined(18), new Defined(1),new Defined(10), new Defined(1), new Defined(17), new Defined(12), new Defined()},
        {new Defined(), new Defined(), new Defined(), new Defined(18), new Defined(), new Defined(),new Defined()},
        {new Defined(),new Defined(), new Defined(), new Defined(1), new Defined(), new Defined(), new Defined()},
        {new Defined(), new Defined(), new Defined(), new Defined(17), new Defined(), new Defined(), new Defined()},
       
    };
    for(int i = 0; i < 7; i ++){
        for(int j = 0; j < 7; j++)
            board[i][j] = tempt[i][j];
    }
    // std::deque<std::deque<Defined>> terrain;
    Defined temp = Defined();
    
    // CONNECT THE BOARD:
    for (int i = 0; i < 7; i ++){
        for (int j = 0; j < 7; j ++){
            Defined* xe = board[i][j];
            if(i>0){
                xe->up = board[i-1][j];
            }
            if(j>0){
                xe->left = board[i][j-1];
            }
            if(i<6){
                xe->down = board[i+1][j];
            }
            if(j<6){
                xe->right = board[i][j+1];
            }
        
            
        }

    }

    
    dq.push_back(board[3][3]);
    int max = 100000;
    Defined* max_node = nullptr;
    while(!dq.empty()){
        std::cout<<"Constraining"<<std::endl;
        Defined* curr = dq.front();
        dq.pop_front();
        if(curr == nullptr) continue;
        if(curr->empt) continue;
        if(curr->constrained) continue;
        
        int ret = remove_options(curr,0,0);
        if(ret%100<max){
            max = ret;
            if(ret/100 == 0){
                max_node = curr->up;
                
            }else if(ret/100 == 1){
                max_node = curr->right;
            }else if(ret/100 == 2){
                max_node = curr->down;
            }else if(ret/100 == 3) {
                max_node = curr->left;
            }
        }
        curr->constrained = true;
        dq.push_back(curr->down);
        dq.push_back(curr->up);
        dq.push_back(curr->left);
        dq.push_back(curr->right);
    }
    constrain(max_node, max);
    for (int i = 0; i < 7; i ++){
        for (int j = 0; j < 7; j ++){
            if(board[i][j]->empt) dq.push_back(board[i][j]);
            
        }

    }
    
    assign_all(&dq);
    
    
    
    // while(true){
    //     visualise(board, 7,7);
    //     int inp = 0;
        

    //     if(inp<1 || inp > 4) continue;
    //     if(inp == 1){
    //         move_u(board,&dq);
    //         move_u(board,&dq);
    //         assign_all(&dq);
    //     }else if(inp == 2){
    //         move_r(board,&dq);
    //         move_r(board,&dq);
    //         assign_all(&dq);
    //     }else if(inp == 3){
    //         move_d(board,&dq);
    //         move_d(board,&dq);
    //         assign_all(&dq);
    //     }else if(inp == 4){
    //         move_l(board,&dq);
    //         move_l(board,&dq);
    //         assign_all(&dq);
    //     }
        
    // }
    // std::cout<<std::endl;
    // std::cout<<max;
    
}

