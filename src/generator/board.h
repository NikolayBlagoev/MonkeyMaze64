#ifndef _BOARD_H_
#define _BOARD_H_

#include <gameplay/enemy_camera.h>
#include <generator/node.h>
#include <render/bezier.h>
#include <render/lighting.h>
#include <render/mesh_tree.h>
#include <render/particle.h>
#include <utils/constants.h>
#include <array>
#include <utility>

typedef std::pair<GPUMesh*, const HitBox&> MeshHitBoxRefs;
typedef std::array<std::array<MeshTree*, utils::TILES_PER_ROW>, utils::TILES_PER_ROW> HeptaGrid;

struct InitialState {
    // Models and corresponding hitboxes
    MeshHitBoxRefs crossing;
    MeshHitBoxRefs room;
    MeshHitBoxRefs tjunction;
    MeshHitBoxRefs tunnel;
    MeshHitBoxRefs turn;
    MeshHitBoxRefs camera;
    MeshHitBoxRefs aperture;
    MeshHitBoxRefs stand1;
    MeshHitBoxRefs stand2;
    MeshHitBoxRefs suzanne;
    MeshHitBoxRefs floor;
    MeshHitBoxRefs pillarBL;
    MeshHitBoxRefs pillarBR;
    MeshHitBoxRefs pillarTL;
    MeshHitBoxRefs pillarTR;
    MeshHitBoxRefs wallHR;
    MeshHitBoxRefs wallHB;
    MeshHitBoxRefs wallHT;
    MeshHitBoxRefs wallFB;
    MeshHitBoxRefs wallFR;
    MeshHitBoxRefs wallFT;
    
    // External objects
    BezierCurveManager& bezierCurveManager;
    LightManager& lightManager;
    std::vector<std::weak_ptr<EnemyCamera>>& cameras;
    std::vector<std::weak_ptr<MeshTree>>& monkeyHeads;
};

class Board {
public:
    Board(Defined*** boardCopy, const InitialState& initialState) { load(boardCopy, initialState, 0UL, 7UL, 0UL, 7UL); }

    void fix_translations();
    void addObjectsRoom(MeshTree* room, Defined* roomTile, const InitialState& initialState);
    void load(Defined*** boardCopy, const InitialState& initialState, size_t startI, size_t stopI, size_t startY, size_t stopY);

    void shiftLeft(LightManager& lightManager,  ParticleEmitterManager& particleEmitterManager);
    void shiftDown(LightManager& lightManager,  ParticleEmitterManager& particleEmitterManager);
    void shiftRight(LightManager& lightManager, ParticleEmitterManager& particleEmitterManager);
    void shiftUp(LightManager& lightManager,    ParticleEmitterManager& particleEmitterManager);

    HeptaGrid board;
};

#endif
