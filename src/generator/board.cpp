#include "board.h"

#include <iostream>

void Board::fix_translations() {
    for (size_t i = 0UL; i < utils::TILES_PER_ROW; i ++){
            for (size_t j = 0UL; j < utils::TILES_PER_ROW; j++) {
                board[i][j]->transform.translate = glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j);
            }
    }
}

void Board::addObjectsRoom(MeshTree* room, Defined* roomTile, const InitialState& initialState) {
    for (size_t i = 0; i < roomTile->objs.size(); i++){
        if (roomTile->objs.at(i)->type == SpecialObjType::EnemyCamera) {
            // Construct camera as a hierarchy of meshes
            MeshTree* retRoot = new MeshTree(
                "stand1", initialState.stand1.second, initialState.stand1.first,
                glm::vec3(-9.9f, 9.0f, 0.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, 90.0f),
                glm::vec3(1.0f));
            MemoryManager::addEl(retRoot);
            MeshTree* standChild = new MeshTree(
                "stand2", initialState.stand2.second, initialState.stand2.first,
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                glm::vec3(1.0f));
            MemoryManager::addEl(standChild);
            retRoot->addChild(standChild->shared_from_this());
            MeshTree* cameraChild = new MeshTree(
                "camera", initialState.camera.second, initialState.camera.first,
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, -40.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                glm::vec3(1.0f));
            MemoryManager::addEl(cameraChild);
            standChild->addChild(cameraChild->shared_from_this());
            MeshTree* apertureChild = new MeshTree(
                "aperture", initialState.aperture.second, initialState.aperture.first,
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                glm::vec3(1.0f));
            MemoryManager::addEl(apertureChild);
            cameraChild->addChild(apertureChild->shared_from_this());
            // Create and set parameters of area light representing camera cone of vision
            apertureChild->al                           = initialState.lightManager.addAreaLight(glm::vec3(1.0f, -3.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            apertureChild->al->falloff                  = utils::CONSTANT_AREA_LIGHT_FALLOFF;
            apertureChild->al->intensityMultiplier      = 2.0f;
            apertureChild->al->externalRotationControl  = true;
            EnemyCamera* cam = new EnemyCamera(&(cameraChild->transform.selfRotate), &(standChild->transform.selfRotate), &(apertureChild->al->externalForward), &(apertureChild->al->position), &(apertureChild->al->color));
                room->addChild(retRoot->shared_from_this());
                apertureChild->enemyCam = std::shared_ptr<EnemyCamera>(cam);
                initialState.cameras.push_back(apertureChild->enemyCam);
            
            
        } else if (roomTile->objs.at(i)->type == SpecialObjType::Collectible) {
            MeshTree* retRoot = new MeshTree(
                "suzanne", initialState.suzanne.second, initialState.suzanne.first,
                glm::vec3(-3.0f, 2.0f, 0.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                glm::vec3(1.0f));
            MemoryManager::addEl(retRoot);
            room->addChild(retRoot->shared_from_this());
            BezierCurve<glm::vec3> b3d              = BezierCurve<glm::vec3>(glm::vec3(-3.f, 2.f, 0.f), glm::vec3(-3.3f , 2.5f, 0.f), glm::vec3(-2.7f , 3.f, 0.f), glm::vec3(-3.f, 3.5f, 0.f), 10.f);
                BezierCurve<glm::vec3> b3d2             = BezierCurve<glm::vec3>(glm::vec3(-3.f, 3.5f, 0.f), glm::vec3(-2.7f , 3.f, 0.f), glm::vec3(-3.3f , 2.5f, 0.f), glm::vec3(-3.f, 2.f, 0.f), 10.f);
                BezierComposite<glm::vec3> b3c          = BezierComposite<glm::vec3>({b3d, b3d2}, true, 20.f);
                BezierComboComposite<glm::vec3> combo   = BezierComboComposite<glm::vec3>(b3c, &retRoot->transform.translate, retRoot->shared_from_this());
                initialState.bezierCurveManager.add3dComposite(combo);

                BezierCurve<glm::vec4> b4d              = BezierCurve<glm::vec4>(glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(0.f , 0.3826834f, 0.f, 0.9238795f), glm::vec4(0.f , 0.7132504f, 0.f, 0.7009093f), glm::vec4(0.f , 1.f, 0.f, 0.f), 10.f);
                BezierCurve<glm::vec4> b4d2             = BezierCurve<glm::vec4>(glm::vec4(0.f , 1.f, 0.f, 0.f) , glm::vec4(0.f , -0.7132504f, 0.f, 0.7009093f), glm::vec4(0.f , -0.3826834f, 0.f, 0.9238795f),  glm::vec4(0.f, -0.0005f, 0.f, 0.9999999f), 10.f);
                BezierComposite<glm::vec4> b4c          = BezierComposite<glm::vec4>({b4d, b4d2}, true, 20.f);
                BezierComboComposite<glm::vec4> combo2  = BezierComboComposite<glm::vec4>(b4c, &retRoot->transform.selfRotate, retRoot->shared_from_this());
                initialState.bezierCurveManager.add4dComposite(combo2);
            initialState.monkeyHeads.push_back(retRoot->shared_from_this());
        }
    }
}

void Board::load(Defined*** boardCopy, const InitialState& initialState, size_t startI, size_t stopI, size_t startY, size_t stopY) {
    for (size_t i = startI; i < stopI; i++) {
            for (size_t j = startY; j < stopY; j++) {
                switch (boardCopy[i][j]->tileType) {
                    case TileType::INVALID: {
                        std::cerr << "Invalid tile loaded" << std::endl;
                        break;
                    } case TileType::CROSSING: {
                        MeshTree* crossingTile = new MeshTree(
                            "cross", {}, nullptr,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        board[i][j] = crossingTile;
                        MemoryManager::addEl(crossingTile);

                        MeshTree* pillarTL = new MeshTree(
                            "pillarTL", initialState.pillarTL.second, initialState.pillarTL.first);
                        
                        MemoryManager::addEl(pillarTL);
                        crossingTile->addChild(pillarTL->shared_from_this());

                        MeshTree* pillarBL = new MeshTree(
                            "pillarBL", initialState.pillarBL.second, initialState.pillarBL.first);
                        
                        MemoryManager::addEl(pillarBL);
                        crossingTile->addChild(pillarBL->shared_from_this());

                        MeshTree* pillarBR = new MeshTree(
                            "pillarBL", initialState.pillarBR.second, initialState.pillarBR.first);
                        
                        MemoryManager::addEl(pillarBR);
                        crossingTile->addChild(pillarBR->shared_from_this());

                        MeshTree* pillarTR = new MeshTree(
                            "pillarBL", initialState.pillarTR.second, initialState.pillarTR.first);
                        
                        MemoryManager::addEl(pillarTR);
                        crossingTile->addChild(pillarTR->shared_from_this());


                        MeshTree* floorT = new MeshTree(
                            "floor", initialState.floor.second, initialState.floor.first);
                        
                        MemoryManager::addEl(floorT);
                        crossingTile->addChild(floorT->shared_from_this());

                        break;
                    } case TileType::ROOM1: {
                        MeshTree* roomTile = new MeshTree(
                            "room1", initialState.room.second, initialState.room.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 270.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        board[i][j] = roomTile;
                        MemoryManager::addEl(roomTile);
                        addObjectsRoom(roomTile, boardCopy[i][j], initialState);
                        break;
                    } case TileType::ROOM2: {
                        MeshTree* roomTile = new MeshTree(
                            "room2", initialState.room.second, initialState.room.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        board[i][j] = roomTile;
                        MemoryManager::addEl(roomTile);
                        addObjectsRoom(roomTile, boardCopy[i][j], initialState);
                        break;
                    } case TileType::ROOM3: {
                        MeshTree* roomTile = new MeshTree(
                            "room3", initialState.room.second, initialState.room.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 90.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        board[i][j] = roomTile;
                        MemoryManager::addEl(roomTile);
                        addObjectsRoom(roomTile, boardCopy[i][j], initialState);
                        break;
                    } case TileType::ROOM4: {
                        MeshTree* roomTile = new MeshTree(
                            "room4", initialState.room.second, initialState.room.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 180.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        board[i][j] = roomTile;
                        MemoryManager::addEl(roomTile);
                        addObjectsRoom(roomTile, boardCopy[i][j], initialState);
                        break;
                    } case TileType::EMPTY: {
                        MeshTree* crossingTile = new MeshTree(
                            "empty", initialState.crossing.second, initialState.crossing.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        board[i][j] = crossingTile;
                        MemoryManager::addEl(crossingTile);
                        break;
                    } case TileType::TUNNEL1: {
                        board[i][j] = new MeshTree(
                            "tunnel1", initialState.tunnel.second, initialState.tunnel.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 90.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        MemoryManager::addEl(board[i][j]);
                        break;
                    } case TileType::TUNNEL2: {
                        board[i][j] = new MeshTree(
                            "tunnel2", initialState.tunnel.second, initialState.tunnel.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 180.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        MemoryManager::addEl(board[i][j]);
                        break;
                    } case TileType::TUNNEL3: {
                        board[i][j] = new MeshTree(
                            "tunnel3", initialState.tunnel.second, initialState.tunnel.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 270.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        MemoryManager::addEl(board[i][j]);
                        break;
                    } case TileType::TUNNEL4: {
                        board[i][j] = new MeshTree(
                            "tunnel4", initialState.tunnel.second, initialState.tunnel.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        MemoryManager::addEl(board[i][j]);
                        break;
                    } case TileType::TURN1: {
                        board[i][j] = new MeshTree(
                            "turn1", initialState.turn.second, initialState.turn.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 180.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        MemoryManager::addEl(board[i][j]);
                        break;
                    } case TileType::TURN2: {
                        board[i][j] = new MeshTree(
                            "turn2", initialState.turn.second, initialState.turn.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 270.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        MemoryManager::addEl(board[i][j]);
                        break;
                    } case TileType::TURN3: {
                        board[i][j] = new MeshTree(
                            "turn3", initialState.turn.second, initialState.turn.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        MemoryManager::addEl(board[i][j]);
                        break;
                    } case TileType::TURN4: {
                        board[i][j] = new MeshTree(
                            "turn4", initialState.turn.second, initialState.turn.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 90.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        MemoryManager::addEl(board[i][j]);
                        break;
                    } case TileType::TJUNCTION1: {
                        board[i][j] = new MeshTree(
                            "tjunction1", initialState.tjunction.second, initialState.tjunction.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 270.0f),
                            glm::vec4(0.f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        MemoryManager::addEl(board[i][j]);
                        break;
                    } case TileType::TJUNCTION2: {
                        board[i][j] = new MeshTree(
                            "tjunction2", initialState.tjunction.second, initialState.tjunction.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        MemoryManager::addEl(board[i][j]);
                        break;
                    } case TileType::TJUNCTION3: {
                        board[i][j] = new MeshTree(
                            "tjunction3", initialState.tjunction.second, initialState.tjunction.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 90.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        MemoryManager::addEl(board[i][j]);
                        break;
                    } case TileType::TJUNCTION4: {
                        board[i][j] = new MeshTree(
                            "tjunction4", initialState.tjunction.second, initialState.tjunction.first,
                            glm::vec3(utils::TILE_LENGTH_X * i, 0.0f, utils::TILE_LENGTH_Z * j),
                            glm::vec4(0.0f, 1.0f, 0.0f, 180.0f),
                            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            glm::vec3(0.3f));
                        MemoryManager::addEl(board[i][j]);
                        break;
                    }
                }
            }
        }
    fix_translations();
}

void Board::shiftLeft(LightManager& lightManager, ParticleEmitterManager& particleEmitterManager) {
    // We shift two columns to the left
    constexpr size_t lastColumn         = utils::TILES_PER_ROW - 1UL;
    constexpr size_t secondToLastColumn = utils::TILES_PER_ROW - 2UL;
    constexpr size_t secondColumn       = 1UL;

    // Remove old tiles
    for (size_t i = 0UL; i < utils::TILES_PER_ROW; i++) {
        for (size_t j = lastColumn; j >= secondToLastColumn; j--) {
            board[i][j]->clean(lightManager, particleEmitterManager);
            MemoryManager::removeEl(board[i][j]);
        }
    }

    // Shift unremoved tiles leftwards
    for (size_t i = 0UL; i < utils::TILES_PER_ROW; i++) {
        for (size_t j = lastColumn; j > secondColumn; j--) {
            board[i][j] = board[i][j - 2UL];
        }
    }
}

void Board::shiftDown(LightManager& lightManager, ParticleEmitterManager& particleEmitterManager) {
    // We shift two rows down
    constexpr size_t thirdRow        = 2UL;
    constexpr size_t secondToLastRow = utils::TILES_PER_ROW - 2UL;

    // Remove old tiles
    for (size_t j = 0UL; j < utils::TILES_PER_ROW; j++) {
        for (size_t i = 0UL; i < thirdRow; i++) {
            board[i][j]->clean(lightManager, particleEmitterManager);
            MemoryManager::removeEl(board[i][j]);
        }
    }

    // Shift unremoved tiles downwards
    for (size_t j = 0UL; j < utils::TILES_PER_ROW; j++) {
        for (size_t i = 0UL; i < secondToLastRow; i++) {
            board[i][j] = board[i + 2UL][j];
        }
    }
}

void Board::shiftRight(LightManager& lightManager, ParticleEmitterManager& particleEmitterManager) {
    // We shift two columns to the right
    constexpr size_t thirdColumn        = 2UL;
    constexpr size_t secondToLastColumn = utils::TILES_PER_ROW - 2UL;

    for (size_t i = 0UL; i < utils::TILES_PER_ROW; i++) {
        for (size_t j = 0UL; j < thirdColumn; j++) {
            board[i][j]->clean(lightManager, particleEmitterManager);
            MemoryManager::removeEl(board[i][j]);
        }
    }

    // Shift unremoved tiles rightwards
    for(size_t i = 0UL; i < utils::TILES_PER_ROW; i++) {
        for(size_t j = 0UL; j < secondToLastColumn; j++) {
            board[i][j] = board[i][j + 2UL];
        }
    }
}

void Board::shiftUp(LightManager& lightManager, ParticleEmitterManager& particleEmitterManager) {
    // We shift two rows up
    constexpr size_t lastRow            = utils::TILES_PER_ROW - 1UL;
    constexpr size_t secondToLastRow    = utils::TILES_PER_ROW - 2UL;
    constexpr size_t secondRow          = 1UL;

    // Remove old tiles
    for (size_t j = 0UL; j < 7UL; j++) {
        for (size_t i = 6; i >= secondToLastRow; i--) {
            board[i][j]->clean(lightManager, particleEmitterManager);
            MemoryManager::removeEl(board[i][j]);                
        }
    }

    // Shift unremoved tiles upwards
    for (size_t j = 0UL; j < utils::TILES_PER_ROW; j++) {
        for (size_t i = lastRow; i > secondRow; i--) {
            board[i][j] = board[i - 2UL][j];
        }
    }
}
