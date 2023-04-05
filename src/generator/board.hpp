#ifndef _BOARD_HPP_
#define _BOARD_HPP_

#include <gameplay/enemy_camera.h>
#include <generator/node.h>
#include <render/bezier.h>
#include <render/lighting.h>
#include <render/mesh_tree.h>
#include <iostream>
struct Pass {
    Mesh* crossing;
    Mesh* room;
    Mesh* tjunction;
    Mesh* tunnel;
    Mesh* turn;
    Mesh* camera;
    Mesh* aperture;
    Mesh* stand2;
    Mesh* stand1;
    Mesh* suzanne;
    BezierCurveManager& rndrr;
    LightManager& lightManager;
    std::vector<std::weak_ptr<EnemyCamera>>& cameras;
    std::vector<std::weak_ptr<MeshTree>>& monkeyHeads;
};

class Board {
public:
    const float factorx = 7.72f;
    const float factory = 7.72f;
    MeshTree* board[7][7];

    Board(Defined*** boardCopy, Pass p) { load(boardCopy,p, 0, 7, 0, 7); }

    void load(Defined*** boardCopy, Pass p, int startI, int stopI, int startY, int stopY){
        for(int i = startI; i < stopI; i ++){
                for(int j = startY; j < stopY; j++){
                    if(boardCopy[i][j]->tileType == 1 ){
                        MeshTree* crossingTile = new MeshTree("cross", p.crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        board[i][j] = crossingTile;
                        MemoryManager::addEl(crossingTile);
                    }else if(boardCopy[i][j]->tileType == 2 ){
                        MeshTree* roomTile = new MeshTree("room", p.room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        board[i][j] =roomTile;
                        MemoryManager::addEl(roomTile);
                        addObjectsRoom(roomTile, boardCopy[i][j], p.aperture, p.camera, p.stand2, 
                        p.stand1, p.suzanne, p.rndrr, p.lightManager, p.cameras, p.monkeyHeads);
                        
                    }else if(boardCopy[i][j]->tileType == 3){
                        MeshTree* roomTile = new MeshTree("room",p.room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        board[i][j] =roomTile;
                        MemoryManager::addEl(roomTile);
                        addObjectsRoom(roomTile, boardCopy[i][j], p.aperture, p.camera, p.stand2, p.stand1, p.suzanne, p.rndrr, p.lightManager, p.cameras, p.monkeyHeads);
                    }else if(boardCopy[i][j]->tileType == 4){
                        MeshTree* roomTile = new MeshTree("room",p.room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        board[i][j] =roomTile;
                        MemoryManager::addEl(roomTile);
                        addObjectsRoom(roomTile, boardCopy[i][j], p.aperture, p.camera, p.stand2, p.stand1, p.suzanne, p.rndrr, p.lightManager, p.cameras, p.monkeyHeads);
                    }else if(boardCopy[i][j]->tileType == 5){
                        MeshTree* roomTile = new MeshTree("room",p.room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        board[i][j] =roomTile;
                        MemoryManager::addEl(roomTile);
                        addObjectsRoom(roomTile, boardCopy[i][j], p.aperture, p.camera, p.stand2, p.stand1, p.suzanne, p.rndrr, p.lightManager, p.cameras, p.monkeyHeads);
                    }else if(boardCopy[i][j]->tileType == 6){
                        MeshTree* crossingTile = new MeshTree("empty", p.crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        board[i][j] =crossingTile;
                        MemoryManager::addEl(crossingTile);
                    }else if(boardCopy[i][j]->tileType == 7){
                        board[i][j] =(new MeshTree("tunne", p.tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                        MemoryManager::addEl(board[i][j]);
                    }else if(boardCopy[i][j]->tileType == 8){
                        board[i][j] =(new MeshTree("tunne", p.tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                        MemoryManager::addEl(board[i][j]);
                    }else if(boardCopy[i][j]->tileType == 9){
                        board[i][j] =(new MeshTree("tunne", p.tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                        MemoryManager::addEl(board[i][j]);
                    }else if(boardCopy[i][j]->tileType == 10){
                        board[i][j] =(new MeshTree("tunne", p.tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                        MemoryManager::addEl(board[i][j]);
                    }else if(boardCopy[i][j]->tileType == 11){
                        board[i][j] =(new MeshTree("turn", p.turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                        MemoryManager::addEl(board[i][j]);
                    }else if(boardCopy[i][j]->tileType == 12){
                        board[i][j] =(new MeshTree("turn",p.turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                        MemoryManager::addEl(board[i][j]);
                    }else if(boardCopy[i][j]->tileType == 13){
                        board[i][j] =(new MeshTree("turn", p.turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f),  glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                        MemoryManager::addEl(board[i][j]);
                    }else if(boardCopy[i][j]->tileType == 14){
                        board[i][j] =(new MeshTree("turn", p.turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                        MemoryManager::addEl(board[i][j]);
                    }else if(boardCopy[i][j]->tileType == 15){
                        board[i][j] =(new MeshTree("tjunction", p.tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                        MemoryManager::addEl(board[i][j]);
                    }else if(boardCopy[i][j]->tileType == 16){
                        board[i][j] =(new MeshTree("tjunction", p.tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                        MemoryManager::addEl(board[i][j]);
                    }else if(boardCopy[i][j]->tileType == 17){
                        board[i][j] =(new MeshTree("tjunction", p.tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                        MemoryManager::addEl(board[i][j]);
                    }else if(boardCopy[i][j]->tileType == 18){
                        board[i][j] =(new MeshTree("tjunction", p.tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                        MemoryManager::addEl(board[i][j]);
                    } 
                }
            }
            fix_translations();
    }

    void fix_translations(){
        for(int i = 0; i < 7; i ++){
                for(int j = 0; j < 7; j++){
                    board[i][j]->transform.translate =  glm::vec3(factorx*i, 0.f, factory*j);
                }
        }
    }

    void move_l(LightManager& lightManager) {
        for(int i = 0; i < 7; i ++){
            for(int j = 6; j >= 5 ; j--){
                if(board[i][j]->al!=nullptr){}
                board[i][j]->clean(lightManager);
                MemoryManager::removeEl(board[i][j]);
                
            }
        }
        for(int i = 0; i < 7; i ++){
            for(int j = 6; j > 1 ; j--){
                board[i][j] = board[i][j-2];
            }
        }
    }

    void move_d(LightManager& lightManager) {
        for(int j = 0; j < 7; j ++){
            for(int i = 0; i < 2; i++){
                if(board[i][j]->al!=nullptr){}
                board[i][j]->clean(lightManager);
                MemoryManager::removeEl(board[i][j]);
                
            }
        }
        for(int j = 0; j < 7; j ++){
            for(int i = 0; i < 5; i++){
                board[i][j] = board[i+2][j];
            }
        }
    }

    void move_r(LightManager& lightManager){
        for(int i = 0; i < 7; i ++){
            for(int j = 0; j < 2; j++){
                if(board[i][j]->al!=nullptr){}
                board[i][j]->clean(lightManager);
                MemoryManager::removeEl(board[i][j]);
                
            }
        }
        for(int i = 0; i < 7; i ++){
            for(int j = 0; j < 5; j++){
                board[i][j] = board[i][j+2];
            }
        }

    }

    void move_u(LightManager& lightManager){
        for(int j = 0; j < 7; j ++){
            for(int i = 6; i >= 5; i--){
                if(board[i][j]->al!=nullptr){}
                board[i][j]->clean(lightManager);
                MemoryManager::removeEl(board[i][j]);
                
            }
        }
        for(int j = 0; j < 7; j ++){
            for(int i = 6; i > 1 ; i--){
                board[i][j] = board[i-2][j];
            }
        }
        
    }

    void addObjectsRoom(MeshTree* room, Defined* roomTile, Mesh* aperture, Mesh* camera, Mesh* stand2, Mesh* stand1, Mesh* suzanne, BezierCurveManager& rndrr, LightManager& lightManager, std::vector<std::weak_ptr<EnemyCamera>>& cameras, std::vector<std::weak_ptr<MeshTree>>& monkeyHeads){
        for(int i = 0; i < roomTile->objs.size(); i++){
            if(roomTile->objs.at(i)->type == SpecialObjType::EnemyCamera){
                MeshTree* ret = new MeshTree("stand1", stand1, glm::vec3(-9.9f, 9.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec3(1.f));
                MemoryManager::addEl(ret);
                MeshTree* stand2m = new MeshTree("stand2",stand2, glm::vec3(0.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(1.f));
                MemoryManager::addEl(stand2m);
                ret->addChild(stand2m->shared_from_this());
                MeshTree* cameram = new MeshTree("cam",camera, glm::vec3(0.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, -40.f), glm::vec3(1.f));
                MemoryManager::addEl(cameram);
                stand2m->addChild(cameram->shared_from_this());
                MeshTree* aperturem = new MeshTree("ap",aperture, glm::vec3(0.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(1.f));
                MemoryManager::addEl(aperturem);
                cameram->addChild(aperturem->shared_from_this());
                aperturem->al=lightManager.addAreaLight(glm::vec3(1.0f, -3.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                aperturem->al->falloff = utils::CONSTANT_AREA_LIGHT_FALLOFF;
                aperturem->al->intensityMultiplier = 2.f;
                aperturem->al->externalRotationControl = true;
                EnemyCamera* cam = new EnemyCamera(&(cameram->transform.selfRotate), &(stand2m->transform.selfRotate), &(aperturem->al->externalForward), &(aperturem->al->position), &(aperturem->al->color));
                room->addChild(ret->shared_from_this());
                aperturem->enemyCam = std::shared_ptr<EnemyCamera>(cam);
                cameras.push_back(aperturem->enemyCam);
                aperturem->al=lightManager.addAreaLight(glm::vec3(1.0f, -3.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                aperturem->al->falloff = utils::CONSTANT_AREA_LIGHT_FALLOFF;
                aperturem->al->intensityMultiplier = 2.f;
                aperturem->al->externalRotationControl = true;
            }else if(roomTile->objs.at(i)->type == SpecialObjType::Collectible){
                MeshTree* ret = new MeshTree("suzanne", suzanne, glm::vec3(-3.f, 2.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(1.f), false);
                MemoryManager::addEl(ret);
                
                room->addChild(ret->shared_from_this());
                monkeyHeads.push_back(ret->shared_from_this());
                BezierCurve<glm::vec3> b3d              = BezierCurve<glm::vec3>(glm::vec3(-3.f, 2.f, 0.f), glm::vec3(-3.3f , 3.f, 0.f), glm::vec3(-2.7f , 4.f, 0.f), glm::vec3(-3.f, 5.f, 0.f), 10.f);
                BezierCurve<glm::vec3> b3d2             = BezierCurve<glm::vec3>(glm::vec3(-3.f, 5.f, 0.f), glm::vec3(-2.7f , 4.f, 0.f), glm::vec3(-3.3f , 3.f, 0.f), glm::vec3(-3.f, 2.f, 0.f), 10.f);
                BezierComposite<glm::vec3> b3c          = BezierComposite<glm::vec3>({b3d, b3d2}, true, 20.f);
                BezierComboComposite<glm::vec3> combo   = BezierComboComposite<glm::vec3>(b3c, &ret->transform.translate, ret->shared_from_this());
                rndrr.add3dComposite(combo);

                BezierCurve<glm::vec4> b4d              = BezierCurve<glm::vec4>(glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(0.f , 0.3826834f, 0.f, 0.9238795f), glm::vec4(0.f , 0.7132504f, 0.f, 0.7009093f), glm::vec4(0.f , 1.f, 0.f, 0.f), 10.f);
                BezierCurve<glm::vec4> b4d2             = BezierCurve<glm::vec4>(glm::vec4(0.f , 1.f, 0.f, 0.f) , glm::vec4(0.f , -0.7132504f, 0.f, 0.7009093f), glm::vec4(0.f , -0.3826834f, 0.f, 0.9238795f),  glm::vec4(0.f, -0.0005f, 0.f, 0.9999999f), 10.f);
                BezierComposite<glm::vec4> b4c          = BezierComposite<glm::vec4>({b4d, b4d2}, true, 20.f);
                BezierComboComposite<glm::vec4> combo2  = BezierComboComposite<glm::vec4>(b4c, &ret->transform.selfRotate, ret->shared_from_this());
                rndrr.add4dComposite(combo2);
            }
        }
    }
};

#endif
