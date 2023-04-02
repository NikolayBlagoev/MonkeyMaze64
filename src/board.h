#include <render/mesh_tree.h>
#include <render/bezier.h>
struct Pass{
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
    BezierCurveRenderer& rndrr;
    LightManager& lightManager;
    std::vector<std::weak_ptr<CameraObj>>& cameras;

};
class Board{
  public:
    const float factorx = 7.72f;
    const float factory = 7.72f;
    MeshTree* board[7][7];
    Board(Defined*** boardCopy, Pass p){
        load(boardCopy,p, 0, 7, 0, 7);
    }

    void load(Defined*** boardCopy, Pass p, int startI, int stopI, int startY, int stopY){
        for(int i = startI; i < stopI; i ++){
                // glm::vec3 offset(0.f,10.f*i,0.f);
                for(int j = startY; j < stopY; j++){
              
                    if(boardCopy[i][j]->tileType == 1 ){
                        MeshTree* crossingTile = new MeshTree(p.crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        board[i][j] = crossingTile;
                    }else if(boardCopy[i][j]->tileType == 2 ){
                        MeshTree* roomTile = new MeshTree(p.room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        board[i][j] =roomTile;
                        addObjectsRoom(roomTile, boardCopy[i][j], p.aperture, p.camera, p.stand2, 
                        p.stand1, p.suzanne, p.rndrr, p.lightManager, p.cameras);
                    }else if(boardCopy[i][j]->tileType == 3){
                        MeshTree* roomTile = new MeshTree(p.room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        board[i][j] =roomTile;
                        addObjectsRoom(roomTile, boardCopy[i][j], p.aperture, p.camera, p.stand2, p.stand1, p.suzanne, p.rndrr, p.lightManager, p.cameras);
                    }else if(boardCopy[i][j]->tileType == 4){
                        MeshTree* roomTile = new MeshTree(p.room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        board[i][j] =roomTile;
                        addObjectsRoom(roomTile, boardCopy[i][j], p.aperture, p.camera, p.stand2, p.stand1, p.suzanne, p.rndrr, p.lightManager, p.cameras);
                    }else if(boardCopy[i][j]->tileType == 5){
                        MeshTree* roomTile = new MeshTree(p.room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        board[i][j] =roomTile;
                        addObjectsRoom(roomTile, boardCopy[i][j], p.aperture, p.camera, p.stand2, p.stand1, p.suzanne, p.rndrr, p.lightManager, p.cameras);
                    }else if(boardCopy[i][j]->tileType == 6){
                        MeshTree* crossingTile = new MeshTree(p.crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        board[i][j] =crossingTile;
                    }else if(boardCopy[i][j]->tileType == 7){
                        board[i][j] =(new MeshTree(p.tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 8){
                        board[i][j] =(new MeshTree(p.tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 9){
                        board[i][j] =(new MeshTree(p.tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 10){
                        board[i][j] =(new MeshTree(p.tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 11){
                        board[i][j] =(new MeshTree(p.turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 12){
                        board[i][j] =(new MeshTree(p.turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 13){
                        board[i][j] =(new MeshTree(p.turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f),  glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 14){
                        board[i][j] =(new MeshTree(p.turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 15){
                        board[i][j] =(new MeshTree(p.tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 16){
                        board[i][j] =(new MeshTree(p.tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 17){
                        board[i][j] =(new MeshTree(p.tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 18){
                        board[i][j] =(new MeshTree(p.tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    } 
                }
            }
    }
    void move_l(LightManager& lightManager){
        for(int i = 0; i < 7; i ++){
            for(int j = 1; j >= 0 ; j--){
                if(board[i][j]->al!=nullptr){}
                board[i][j]->clean(lightManager);
                board[i][j]->~MeshTree();
                free(board[i][j]);
            }
        }
        for(int i = 0; i < 7; i ++){
            for(int j = 6; j > 1 ; j--){
                board[i][j] = board[i][j-2];
            }
        }
    }

    void move_d(LightManager& lightManager){
        for(int j = 0; j < 7; j ++){
            for(int i = 5; i < 7; i++){
                if(board[i][j]->al!=nullptr){}
                board[i][j]->clean(lightManager);
                board[i][j]->~MeshTree();
                free(board[i][j]);
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
            for(int j = 5; j < 7; j++){
                if(board[i][j]->al!=nullptr){}
                board[i][j]->clean(lightManager);
                board[i][j]->~MeshTree();
                free(board[i][j]);
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
            for(int i = 1; i >= 0 ; i--){
                if(board[i][j]->al!=nullptr){}
                board[i][j]->clean(lightManager);
                board[i][j]->~MeshTree();
                free(board[i][j]);
            }
        }
        for(int j = 0; j < 7; j ++){
            for(int i = 6; i > 1 ; i--){
                board[i][j] = board[i-2][j];
            }
        }
        
    }


    void addObjectsRoom(MeshTree* room, Defined* roomTile, Mesh* aperture, Mesh* camera, Mesh* stand2, Mesh* stand1, Mesh* suzanne, BezierCurveRenderer& rndrr, LightManager& lightManager, std::vector<std::weak_ptr<CameraObj>>& cameras){
    for(int i = 0; i < roomTile->objs.size(); i++){
        if(roomTile->objs.at(i)->type == 0){
            // CameraObj* cam = makeCamera(aperture, camera, stand2, stand1, glm::vec3(-9.9f, 9.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec3(1.f));
            MeshTree* ret = new MeshTree(stand1, glm::vec3(-9.9f, 9.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec3(1.f));
            MeshTree* stand2m = new MeshTree(stand2, glm::vec3(0.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(1.f));
            ret->addChild(stand2m->self);
            MeshTree* cameram = new MeshTree(camera, glm::vec3(0.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(1.f));
            stand2m->addChild(cameram->self);
            MeshTree* aperturem = new MeshTree(aperture, glm::vec3(0.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(1.f));
            cameram->addChild(aperturem->self);
            CameraObj* cam = new CameraObj(&(cameram->transform.selfRotate), &(stand2m->transform.selfRotate));
            room->addChild(ret->self);
            room->camera = std::shared_ptr<CameraObj>(cam);
            cameras.push_back(std::weak_ptr(room->camera));
            
            aperturem->al=lightManager.addAreaLight(glm::vec3(1.0f, -3.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            aperturem->al->falloff = 0.f;
            aperturem->al->intensityMultiplier = 2.f;
            aperturem->al->selfRotating = true;
            // aperturem->modelMatrix();
        }else if(roomTile->objs.at(i)->type == 1){
            // CameraObj* cam = makeCamera(aperture, camera, stand2, stand1, glm::vec3(-9.9f, 9.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec3(1.f));
            MeshTree* ret = new MeshTree(suzanne, glm::vec3(-3.f, 2.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(1.f));
            room->addChild(ret->self);
            BezierCurve3d* b3d = new BezierCurve3d(glm::vec3(-3.f, 2.f, 0.f), glm::vec3(-3.3f , 3.f, 0.f), glm::vec3(-2.7f , 4.f, 0.f), glm::vec3(-3.f, 5.f, 0.f), 10.f);
            BezierCurve3d* b3d2 = new BezierCurve3d(glm::vec3(-3.f, 5.f, 0.f), glm::vec3(-2.7f , 4.f, 0.f), glm::vec3(-3.3f , 3.f, 0.f), glm::vec3(-3.f, 2.f, 0.f), 10.f);
            CompositeBezier3d* b3c = new CompositeBezier3d({b3d,b3d2}, true, 20.f);
            BezierCombo3dcomp* combo = new BezierCombo3dcomp(b3c,ret->translate);
            rndrr.add3dcomp(combo);

            BezierCurve4d* b4d = new BezierCurve4d(glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(0.f , 0.3826834f, 0.f, 0.9238795f), glm::vec4(0.f , 0.7132504f, 0.f, 0.7009093f), glm::vec4(0.f , 1.f, 0.f, 0.f), 10.f);
            BezierCurve4d* b4d2 = new BezierCurve4d(glm::vec4(0.f , 1.f, 0.f, 0.f) , glm::vec4(0.f , -0.7132504f, 0.f, 0.7009093f), glm::vec4(0.f , -0.3826834f, 0.f, 0.9238795f),  glm::vec4(0.f, -0.0005f, 0.f, 0.9999999f), 10.f);
            CompositeBezier4d* b4c = new CompositeBezier4d({b4d, b4d2}, true, 20.f);
            BezierCombo4dcomp* combo2 = new BezierCombo4dcomp(b4c,ret->selfRotate);
            rndrr.add4dcomp(combo2);

            
        }
    }
}
};