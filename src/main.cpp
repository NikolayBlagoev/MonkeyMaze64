#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
#include <GLFW/glfw3.h> // Include glad before glfw3
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()
#include <framework/window.h>

#include <gameplay/enemy_camera.h>
#include <generator/board.h>
#include <generator/generator.h>
#include <render/bezier.h>
#include <render/config.h>
#include <render/deferred.h>
#include <render/lighting.h>
#include <render/mesh.h>
#include <render/particle.h>
#include <render/scene.h>
#include <render/texture.h>
#include <ui/camera.h>
#include <ui/menu.h>
#include <utils/constants.h>
#include <utils/cutscene_utils.hpp>
#include <utils/hitbox.hpp>
#include <utils/render_utils.hpp>

#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <vector>
#include <ctime>
#include <chrono>

// Game state
// TODO: Have a separate struct for this if it becomes too much
bool cameraZoomed       = false;
bool playerDetected     = false;
bool xToonPowerUp       = false;
bool cutsceneTriggered  = false;
bool inCutscene         = false;
size_t seizureCounter   = 0UL;
size_t seizureSubCounter   = 0UL;

std::chrono::time_point<std::chrono::high_resolution_clock> playerLastDetected;
std::mutex m;
std::condition_variable cv;
bool ready_ = false;
bool processed_ = false;
std::vector<std::weak_ptr<EnemyCamera>> cameras;
std::vector<std::weak_ptr<MeshTree>> monkeyHeads;
glm::vec3 offsetBoard(-3.0f * utils::TILE_LENGTH_X, 0.0f, -3.0f * utils::TILE_LENGTH_Z);
Defined*** boardCopy;
const float board_init_off = -6.2f;
bool motion = false;
int dir = -1;
bool* ready = new bool;
bool processed = false;
Generator* gen =  new Generator();

void signalChange() {
    {
        ready_ = true;
        std::unique_lock<std::mutex> lk(m);
        cv.notify_all();
    }
    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, []{ return processed_;});
        processed_ = false;
    }
}

void backgroundMazeGeneration() {
    gen->instantiate_terr();
    boardCopy = gen->board;
    gen->visualise(gen->board, 7, 7);
    while (true) {
        //TODO: SAFE ACCESS!!!
        {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, []{ return ready_;});
            if(dir == 1){
                gen->move_u(gen->board, &(gen->dq));
                gen->move_u(gen->board, &(gen->dq));
                
            }else if(dir == 2){
                gen->move_r(gen->board, &(gen->dq));
                gen->move_r(gen->board, &(gen->dq));
            }else if(dir == 3){
                gen->move_d(gen->board, &(gen->dq));
                gen->move_d(gen->board, &(gen->dq));
            }else if(dir == 4){
                gen->move_l(gen->board, &(gen->dq));
                gen->move_l(gen->board, &(gen->dq));
            }else{
                gen->remove_head((dir/10)%10, dir%10);
            }
            gen->visualise(gen->board, 7, 7);
            gen->assign_all(&(gen->dq));
            gen->visualise(gen->board, 7, 7);
            ready_ = false;
            
            boardCopy = gen->board;
            processed_ = true;
            lk.unlock();
            cv.notify_one();
        }
    }
    *ready = false;
}

void onKeyPressed(int key, int) {
    switch (key) {
        case GLFW_KEY_LEFT_CONTROL:
            cameraZoomed = true;
            break;
    }
}

void onKeyReleased(int key, int) {
    switch (key) {
        case GLFW_KEY_LEFT_CONTROL:
            cameraZoomed = false;
            break;

    }
}

void onMouseMove(const glm::dvec2&) {}

void onMouseClicked(int , int ) {}

void onMouseReleased(int , int ) {}

void keyCallback(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) { onKeyPressed(key, mods); }
    else if (action == GLFW_RELEASE) { onKeyReleased(key, mods); }
}

void mouseButtonCallback(int button, int action, int mods) {
    if (action == GLFW_PRESS) { onMouseClicked(button, mods); }
    else if (action == GLFW_RELEASE) { onMouseReleased(button, mods); }
}

void makeCameraMoves(glm::vec3 playerPos, std::chrono::time_point<std::chrono::high_resolution_clock> curr_time){
    bool curr_detected = false;
    for(size_t i = 0; i < cameras.size(); i++){
        if(cameras.at(i).expired()){
            cameras.erase(cameras.begin() + i);
            i--;
            continue;
        }
        std::shared_ptr<EnemyCamera> cam = cameras.at(i).lock();
        // IF PLAYER IN VIEW DO STUFF
        if(!xToonPowerUp  && cam.get()->canSeePoint(playerPos, 15.f)){
            if(!playerDetected){
                playerLastDetected = curr_time;
            }
            *(cam.get()->color) = glm::vec3(1.f,0.f,0.f);
            curr_detected = true;
            // std::cout<<"DETECTED"<<std::endl;
            continue;
        }else{
            *(cam.get()->color) = glm::vec3(0.f,1.f,1.f);
        }
        // OTHERWISE:
        if(cam.get()->motion_left){
            if(cam.get()->camera->w <= -45.f){
                cam.get()->motion_left = false;
                cam.get()->camera->w += 2.f;
            }else{
                cam.get()->camera->w -= 2.f;
            }
        }else{
            if(cam.get()->camera->w >= 45.f){
                cam.get()->motion_left = true;
                cam.get()->camera->w -= 2.f;
            }else{
                cam.get()->camera->w += 2.f;
            }
        }
    }
    playerDetected = curr_detected;
}

int main(int argc, char* argv[]) {
    // Init maze generation thread
    *ready = false;
    std::thread worker(backgroundMazeGeneration);

    // Player position data
    glm::vec3 playerPos             = glm::vec3(0.0f, -0.1f, 1.0f);
    glm::vec3 playerMiddleOffset    = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 playerCameraPos       = playerPos + glm::vec3(0.0f, 1.0f, 1.5f) + playerMiddleOffset;
    glm::vec3 playerLightOffset     = glm::vec3(-0.9f, 2.5f, 1.3f);

    // Init core objects
    RenderConfig renderConfig;
    Window m_window("Final Project", glm::ivec2(utils::WIDTH, utils::HEIGHT), OpenGLVersion::GL46);
    Camera mainCamera(&m_window, renderConfig, glm::vec3(2.0f, 3.0f, 0.0f), -glm::vec3(1.0f, 1.1f, 0.0f));
    Camera playerCamera(&m_window, renderConfig, playerCameraPos, (playerPos - playerCameraPos) + glm::vec3(0.f,1.f,0.f));
    playerCamera.update = &motion;
    TextureManager textureManager;
    Board* b;
    Scene scene;
    LightManager lightManager(renderConfig);
    ParticleEmitterManager particleEmitterManager(renderConfig);
    std::weak_ptr<const Texture> xToonTex = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "toon_map.png");
    DeferredRenderer mainRenderer(utils::WIDTH, utils::HEIGHT, 
                                  renderConfig, scene, lightManager, particleEmitterManager, xToonTex);
    DeferredRenderer minimapRenderer(utils::WIDTH   / static_cast<int32_t>(utils::MINIMAP_SIZE_SCALE),
                                     utils::HEIGHT  / static_cast<int32_t>(utils::MINIMAP_SIZE_SCALE), 
                                     renderConfig, scene, lightManager, particleEmitterManager, xToonTex);
    HeadCount headCount;
    Menu menu(scene, renderConfig, lightManager, particleEmitterManager, mainRenderer, minimapRenderer, headCount);

    // Register UI callbacks
    m_window.registerKeyCallback(keyCallback);
    m_window.registerMouseMoveCallback(onMouseMove);
    m_window.registerMouseButtonCallback(mouseButtonCallback);

    // Load shadow shaders
    utils::initShadowShaders();

    // Constant names for all textures
    constexpr std::string_view textureAlbedo        = "color_map.jpg";
    constexpr std::string_view textureAO            = "ao_map.jpg";
    constexpr std::string_view textureDisplacement  = "displacement_map.jpg";
    constexpr std::string_view textureMetalness     = "metalness_map.jpg";
    constexpr std::string_view textureNormal        = "normal_map_opengl.jpg";
    constexpr std::string_view textureRoughness     = "roughness_map.jpg";

    /********** Texture loading **********/
    // Crystal
    std::filesystem::path crystalTextureFolder = utils::RESOURCES_DIR_PATH / "textures" / "Crystal";
    std::array<std::string, 7UL> crystalColors = { "blue", "green", "purple", "red", "sky", "white", "yellow" };
    std::array<std::weak_ptr<const Texture>, 7UL> albedoCrystals;
    for (size_t textureIdx = 0UL; textureIdx < albedoCrystals.size(); textureIdx++) {
        albedoCrystals[textureIdx] = textureManager.addTexture(crystalTextureFolder / ("color_map_" + crystalColors[textureIdx] + ".jpg"));
    }
    std::weak_ptr<const Texture> aoCrystal              = textureManager.addTexture(crystalTextureFolder / textureAO);
    std::weak_ptr<const Texture> displacementCrystal    = textureManager.addTexture(crystalTextureFolder / textureDisplacement);
    std::weak_ptr<const Texture> metalnessCrystal       = textureManager.addTexture(crystalTextureFolder / textureMetalness);
    std::weak_ptr<const Texture> normalCrystal          = textureManager.addTexture(crystalTextureFolder / textureNormal);
    std::weak_ptr<const Texture> roughnessCrystal       = textureManager.addTexture(crystalTextureFolder / textureRoughness);

    // Glass
    std::filesystem::path glassTextureFolder        = utils::RESOURCES_DIR_PATH / "textures" / "Glass Blocks";
    std::weak_ptr<const Texture> albedoGlass        = textureManager.addTexture(glassTextureFolder / textureAlbedo);
    std::weak_ptr<const Texture> aoGlass            = textureManager.addTexture(glassTextureFolder / textureAO);
    std::weak_ptr<const Texture> displacementGlass  = textureManager.addTexture(glassTextureFolder / textureDisplacement);
    std::weak_ptr<const Texture> normalGlass        = textureManager.addTexture(glassTextureFolder / textureNormal);
    std::weak_ptr<const Texture> roughnessGlass     = textureManager.addTexture(glassTextureFolder / textureRoughness);

    // Stone
    std::filesystem::path stoneTextureFolder        = utils::RESOURCES_DIR_PATH / "textures" / "Mossy Stone";
    std::weak_ptr<const Texture> albedoStone        = textureManager.addTexture(stoneTextureFolder / textureAlbedo);
    std::weak_ptr<const Texture> aoStone            = textureManager.addTexture(stoneTextureFolder / textureAO);
    std::weak_ptr<const Texture> displacementStone  = textureManager.addTexture(stoneTextureFolder / textureDisplacement);
    std::weak_ptr<const Texture> metalnessStone     = textureManager.addTexture(stoneTextureFolder / textureMetalness);
    std::weak_ptr<const Texture> normalStone        = textureManager.addTexture(stoneTextureFolder / textureNormal);
    std::weak_ptr<const Texture> roughnessStone     = textureManager.addTexture(stoneTextureFolder / textureRoughness);

    // Metal
    std::filesystem::path metalTextureFolder        = utils::RESOURCES_DIR_PATH / "textures" / "Rusty Metal";
    std::weak_ptr<const Texture> albedoMetal        = textureManager.addTexture(metalTextureFolder / textureAlbedo);
    std::weak_ptr<const Texture> aoMetal            = textureManager.addTexture(metalTextureFolder / textureAO);
    std::weak_ptr<const Texture> displacementMetal  = textureManager.addTexture(metalTextureFolder / textureDisplacement);
    std::weak_ptr<const Texture> metalnessMetal     = textureManager.addTexture(metalTextureFolder / textureMetalness);
    std::weak_ptr<const Texture> normalMetal        = textureManager.addTexture(metalTextureFolder / textureNormal);
    std::weak_ptr<const Texture> roughnessMetal     = textureManager.addTexture(metalTextureFolder / textureRoughness);

    // Fur
    std::filesystem::path furTextureFolder      = utils::RESOURCES_DIR_PATH / "textures" / "Yeti Fur";
    std::array<std::string, 6UL> hyperFurColors = { "blue", "green", "red", "violet", "white", "yellow"};
    std::array<std::weak_ptr<const Texture>, 6UL> albedoHyperFur;
    for (size_t textureIdx = 0UL; textureIdx < albedoHyperFur.size(); textureIdx++) {
        albedoHyperFur[textureIdx] = textureManager.addTexture(furTextureFolder / ("color_map_" + hyperFurColors[textureIdx] + ".jpg"));
    }
    std::weak_ptr<const Texture> albedoFur          = textureManager.addTexture(furTextureFolder / textureAlbedo);
    std::weak_ptr<const Texture> aoFur              = textureManager.addTexture(furTextureFolder / textureAO);
    std::weak_ptr<const Texture> displacementFur    = textureManager.addTexture(furTextureFolder / textureDisplacement);
    std::weak_ptr<const Texture> normalFur          = textureManager.addTexture(furTextureFolder / textureNormal);
    std::weak_ptr<const Texture> roughnessFur       = textureManager.addTexture(furTextureFolder / textureRoughness);
    /*************************************/

    /********** Model loading and texture setting ************/
    // In-tile objects
    Mesh apertureCPU    = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "aperture.obj"));
    Mesh cameraCPU      = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "camera.obj"));
    Mesh stand1CPU      = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "stand1.obj"));
    Mesh stand2CPU      = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "stand2.obj"));
    Mesh suzanneCPU     = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "suzanne.obj"));
    GPUMesh aperture(apertureCPU);
    aperture.setAlbedo(albedoGlass);
    aperture.setAO(aoGlass);
    aperture.setDisplacement(displacementGlass, true);
    aperture.setNormal(normalGlass);
    aperture.setRoughness(roughnessGlass);
    GPUMesh camera(cameraCPU);
    GPUMesh stand1(stand1CPU);
    GPUMesh stand2(stand2CPU);
    std::array<GPUMesh*, 3> cameraMetalComponents = { &camera, &stand1, &stand2 };
    for (GPUMesh* component : cameraMetalComponents) {
        component->setAlbedo(albedoMetal);
        component->setAO(aoMetal);
        component->setDisplacement(displacementMetal, true);
        component->setMetallic(metalnessMetal);
        component->setNormal(normalMetal);
        component->setRoughness(roughnessMetal);
    }
    GPUMesh suzanne(suzanneCPU);
    suzanne.setAlbedo(albedoCrystals[3]); // TODO: Link to placement logic so which texture is used is varied
    suzanne.setAO(aoCrystal);
    suzanne.setDisplacement(displacementCrystal, true);
    suzanne.setMetallic(metalnessCrystal);
    suzanne.setNormal(normalCrystal);
    suzanne.setRoughness(roughnessCrystal);
    HitBox apertureHitBox   = HitBox::makeHitBox(apertureCPU,   false);
    HitBox cameraHitBox     = HitBox::makeHitBox(cameraCPU,     false);
    HitBox stand1HitBox     = HitBox::makeHitBox(stand1CPU,     false);
    HitBox stand2HitBox     = HitBox::makeHitBox(stand2CPU,     false);
    HitBox suzanneHitbox    = HitBox::makeHitBox(suzanneCPU,    false);

    // Tile meshes
    Mesh crossingCPU   = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "crossing.obj"));
    Mesh roomCPU       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "room.obj"));
    Mesh tjunctionCPU  = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tjunction.obj"));
    Mesh tunnelCPU     = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tunnel.obj"));
    Mesh turnCPU       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "turn.obj"));
    GPUMesh crossing(crossingCPU);
    GPUMesh room(roomCPU);
    GPUMesh tjunction(tjunctionCPU);
    GPUMesh tunnel(tunnelCPU);
    GPUMesh turn(turnCPU);
    HitBox crossingHitBox   = HitBox::makeHitBox(crossingCPU,   false);
    HitBox roomHitBox       = HitBox::makeHitBox(roomCPU,       false);
    HitBox tjunctionHitBox  = HitBox::makeHitBox(tjunctionCPU,  false);
    HitBox tunnelHitBox     = HitBox::makeHitBox(tunnelCPU,     false);
    HitBox turnHitbox       = HitBox::makeHitBox(turnCPU,       false);

    // Tile pieces
    Mesh floorCPU          = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tiles" / "floor.obj"));
    Mesh pillarBLCPU       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tiles" / "pillar_bottom_left.obj"));
    Mesh pillarBRCPU       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tiles" / "pillar-bottom-right.obj"));
    Mesh pillarTLCPU       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tiles" / "pillar-top-left.obj"));
    Mesh pillarTRCPU       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tiles" / "pillar-top-right.obj"));
    Mesh wallHRCPU       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tiles" / "walf-half-right.obj"));
    Mesh wallHBCPU       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tiles" / "wall-half-bottom.obj"));
    Mesh wallHTCPU       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tiles" / "wall-half-top.obj"));
    Mesh wallFBCPU       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tiles" / "wall-full-bottom.obj"));
    Mesh wallFRCPU       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tiles" / "wall-full-right.obj"));
    Mesh wallFTCPU       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tiles" / "wall-full-top.obj"));    
    GPUMesh floorT(floorCPU);
    GPUMesh pillarBL(pillarBLCPU);
    GPUMesh pillarBR(pillarBRCPU);
    GPUMesh pillarTL(pillarTLCPU);
    GPUMesh pillarTR(pillarTRCPU);
    GPUMesh wallHR(wallHRCPU);
    GPUMesh wallHB(wallHBCPU);
    GPUMesh wallHT(wallHTCPU);
    GPUMesh wallFB(wallFBCPU);
    GPUMesh wallFR(wallFRCPU);
    GPUMesh wallFT(wallFTCPU);
    HitBox floorHitbox          = HitBox::makeHitBox(floorCPU,      false);
    HitBox pillarBLHitbox       = HitBox::makeHitBox(pillarBLCPU,   true);
    HitBox pillarBRHitbox       = HitBox::makeHitBox(pillarBRCPU,   true);
    HitBox pillarTLHitbox       = HitBox::makeHitBox(pillarTLCPU,   true);
    HitBox pillarTRHitbox       = HitBox::makeHitBox(pillarTRCPU,   true);
    HitBox wallHRHitbox         = HitBox::makeHitBox(wallHRCPU,     true);
    HitBox wallHBHitbox         = HitBox::makeHitBox(wallHBCPU,     true);
    HitBox wallHTHitbox         = HitBox::makeHitBox(wallHTCPU,     true);
    HitBox wallFBHitbox         = HitBox::makeHitBox(wallFBCPU,     true);
    HitBox wallFRHitbox         = HitBox::makeHitBox(wallFRCPU,     true);
    HitBox wallFTHitbox         = HitBox::makeHitBox(wallFTCPU,     true);

    // Set textures for all tile components
    std::array<GPUMesh*, 16UL> tileMeshes = { &crossing, &room, &tjunction, &tunnel, &turn, &floorT, &pillarBL, &pillarBR,
     &pillarTL, &pillarTR, &wallHR, &wallHB, &wallHT, &wallFB, &wallFR, &wallFT};
    for (GPUMesh* mesh : tileMeshes) {
        mesh->setAlbedo(albedoStone);
        mesh->setAO(aoStone);
        mesh->setDisplacement(displacementStone, true);
        mesh->setMetallic(metalnessStone);
        mesh->setNormal(normalStone);
        mesh->setRoughness(roughnessStone);
    }

    // Player character
    Mesh defaultMonkeyModel = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose0.obj"));
    std::vector<GPUMesh> monkeyPoses;
    std::vector<HitBox> monkeyHitBoxes;
    monkeyPoses.emplace_back(defaultMonkeyModel);
    monkeyHitBoxes.push_back(HitBox::makeHitBox(defaultMonkeyModel, true));
    for (size_t i = 0UL; i < 16UL; i++) {
        // Load model and create mesh and hitbox objects
        if (i != 0UL) {
            auto fileName   = "monkeypose" + std::to_string(i * 2)+ ".obj";
            Mesh cpuMesh    = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / fileName));
            monkeyPoses.emplace_back(cpuMesh);
            monkeyHitBoxes.push_back(HitBox::makeHitBox(cpuMesh, true));
        }
        
        // Link textures
        monkeyPoses[i].setAlbedo(albedoFur);
        monkeyPoses[i].setAO(aoFur);
        monkeyPoses[i].setDisplacement(displacementFur, true);
        monkeyPoses[i].setNormal(normalFur);
        monkeyPoses[i].setRoughness(roughnessFur);
    }
    /**************************************/

    // Add player mesh node
    MeshTree* player = new MeshTree("player", monkeyHitBoxes[0], &monkeyPoses[0], playerPos,
                                    glm::vec4(0.0f, 1.0f, 0.0f, 180.0f),
                                    glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                                    glm::vec3(0.3f));
    MemoryManager::addEl(player);
    scene.addMesh(player->shared_from_this());

    // Add player torch mesh node
    MeshTree* playerLight = new MeshTree("player light", std::nullopt);
    MemoryManager::addEl(playerLight);
    playerLight->transform.translate                = playerLightOffset;
    playerLight->pl                                 = lightManager.addPointLight(glm::vec3(0.0f), glm::vec3(1.0f, 0.5f, 0.0f), 6.0f);
    playerLight->particleEmitter                    = particleEmitterManager.addEmitter(glm::vec3(0.0f));
    playerLight->particleEmitter->m_baseColor       = glm::vec4(0.75f, 0.4f, 0.1f, 0.1f);
    playerLight->particleEmitter->m_baseVelocity    = glm::vec3(0.0f, 0.001f, 0.0f);
    playerLight->particleEmitter->m_baseLife        = 10.0f;
    playerLight->particleEmitter->m_baseSize        = 0.01f;
    player->addChild(playerLight->shared_from_this());

    // Add test lights
    lightManager.addAreaLight(glm::vec3(2.0f, 2.0f, 0.0f), glm::vec3(1.0f), utils::CONSTANT_AREA_LIGHT_FALLOFF, 10.0f,
                              95.0f, 230.0f);
    lightManager.addAreaLight(glm::vec3(0.0f, 2.0f, 2.0f), glm::vec3(1.0f), utils::CONSTANT_AREA_LIGHT_FALLOFF, 10.0f,
                              135.0f, 270.0f);
    lightManager.addAreaLight(glm::vec3(-2.0f, 2.0f, 0.0f), glm::vec3(1.0f), utils::CONSTANT_AREA_LIGHT_FALLOFF, 10.0f,
                              90.0f, 320.0f);
    lightManager.addAreaLight(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3(1.0f), utils::CONSTANT_AREA_LIGHT_FALLOFF, 10.0f,
                              60.0f, 271.0f);

    // Init MeshTree root
    MeshTree* boardRoot = new MeshTree("board root", std::nullopt);
    MemoryManager::addEl(boardRoot);
    boardRoot->transform.translate = offsetBoard;
   
    // Init Bezier curves
    std::chrono::high_resolution_clock timer; 
    BezierCurve<glm::vec3> b3d                      = BezierCurve<glm::vec3>(glm::vec3(0.f), glm::vec3(1.f , 1.f, 0.f), glm::vec3(-1.f , 2.f, 0.f), glm::vec3(0.f, 3.f, 0.f), 10.f);
    BezierCurve<glm::vec3> b3d2                     = BezierCurve<glm::vec3>(glm::vec3(0.f, 3.f, 0.f), glm::vec3(-1.f , 2.f, 0.f), glm::vec3(1.f , 1.f, 0.f), glm::vec3(0.f), 10.f);
    BezierCurve<glm::vec4> b4d                      = BezierCurve<glm::vec4>(glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(0.f , 0.3826834f, 0.f, 0.9238795f), glm::vec4(0.f , 0.7132504f, 0.f, 0.7009093f), glm::vec4(0.f , 1.f, 0.f, 0.f), 10.f);
    BezierComposite<glm::vec3> b3c                  = BezierComposite<glm::vec3>({b3d, b3d2}, true, 20.f);
    std::chrono::time_point millisec_since_epoch    = timer.now();
    BezierCurveManager bezierCurveManager           = BezierCurveManager(millisec_since_epoch);

    // Bezier curve book-keeping
    bool prev_motion                        = motion;
    std::chrono::time_point start_motion    = millisec_since_epoch;
    playerLastDetected                      = millisec_since_epoch;
    glm::vec3 prev_pos                      = playerPos;

    // Create root node of board
    boardRoot = new MeshTree("boardoot", std::nullopt);
    MemoryManager::addEl(boardRoot);
    scene.root->addChild(boardRoot->shared_from_this());

    // Create state object to pass over to the board object
    InitialState InitialState {
        .crossing   = std::make_pair(&crossing, crossingHitBox),
        .room       = std::make_pair(&room, roomHitBox),
        .tjunction  = std::make_pair(&tjunction, tjunctionHitBox),
        .tunnel     = std::make_pair(&tunnel, tunnelHitBox),
        .turn       = std::make_pair(&turn, turnHitbox),
        .camera     = std::make_pair(&camera, cameraHitBox),
        .aperture   = std::make_pair(&aperture, apertureHitBox),
        .stand1     = std::make_pair(&stand1, stand1HitBox),
        .stand2     = std::make_pair(&stand2, stand2HitBox),
        .suzanne    = std::make_pair(&suzanne, suzanneHitbox),
        .floor      = std::make_pair(&floorT, floorHitbox),
        .pillarBL   = std::make_pair(&pillarBL, floorHitbox),
        .pillarBR   = std::make_pair(&pillarBR, pillarBRHitbox),
        .pillarTL   = std::make_pair(&pillarTL, pillarTLHitbox),
        .pillarTR   = std::make_pair(&pillarTR, pillarTRHitbox),
        .wallHR     = std::make_pair(&wallHR, wallHRHitbox),
        .wallHB     = std::make_pair(&wallHB, wallHBHitbox),
        .wallHT     = std::make_pair(&wallHT, wallHTHitbox),
        .wallFB     = std::make_pair(&wallFB, wallFBHitbox),
        .wallFR     = std::make_pair(&wallFR, wallFRHitbox),
        .wallFT     = std::make_pair(&wallFT, wallFTHitbox),
        .bezierCurveManager = bezierCurveManager,
        .lightManager       = lightManager,
        .cameras            = cameras,
        .monkeyHeads        = monkeyHeads
    };
    
    // Init initial set of tiles
    boardRoot->transform.translate = offsetBoard;
    b = new Board(boardCopy, InitialState);
    for(int i = 0; i < 7; i ++){
        for(int j = 0; j < 7; j++)
            boardRoot->addChild(b->board[i][j]->shared_from_this());
    }

    // Main loop
    while (!m_window.shouldClose()) {
        Camera& currentCamera = renderConfig.controlPlayer ? playerCamera : mainCamera;

        // Controls
        ImGuiIO io = ImGui::GetIO();
        m_window.updateInput();
        if (!io.WantCaptureMouse) { // Prevent camera movement when accessing UI elements
            if (renderConfig.controlPlayer) { currentCamera.updateInput(player, scene.root, playerMiddleOffset); }
            else                            { currentCamera.updateInput(); }
        }

        // Handle cutscene
        if (headCount.headsCollected == headCount.headsToCollect) { cutsceneTriggered = true; }
        if (cutsceneTriggered) {
            utils::initCutscene(bezierCurveManager, renderConfig, lightManager, player, playerLight, mainCamera, playerCamera, playerPos);
            inCutscene = true;
        }
        if (inCutscene) {
            monkeyPoses[0].setAlbedo(albedoHyperFur[seizureCounter]);
            if (++seizureSubCounter >= utils::SEIZURE_SUB_LIMIT) {
                seizureSubCounter   = 0UL;
                seizureCounter      = (seizureCounter + 1UL) % albedoHyperFur.size();
            }
        }

        // Handle power-up
        bool xPressed = m_window.isKeyPressed(GLFW_KEY_X);
        if (xPressed != xToonPowerUp) {
            if (!xPressed)  {
                renderConfig.lightingModel = LightingModel::PBR;
                renderConfig.exposure   = 1.0f;
                renderConfig.gamma      = 2.2f;
            }
            if (xPressed)   { 
                renderConfig.lightingModel = LightingModel::XToon;
                renderConfig.exposure   = 0.4f;
                renderConfig.gamma      = 1.0f;
            }
            mainRenderer.initLightingShader();
            minimapRenderer.initLightingShader();
            xToonPowerUp = xPressed;
        }

        playerPos = (player->transform.translate);
        auto test = player->modelMatrix(false) * glm::vec4(glm::vec3(0.0f), 1.0f);
        auto test2 = player->modelMatrix(true) * glm::vec4(glm::vec3(0.0f), 1.0f);

        for (size_t childIdx = 0; childIdx < monkeyHeads.size(); childIdx++) {
            std::weak_ptr<MeshTree> head = monkeyHeads.at(childIdx);
            if (head.expired()) { 
                monkeyHeads.erase(monkeyHeads.begin() + childIdx);
                continue; 
            }
            MeshTree* headMesh = head.lock().get();
            glm::vec4 monkeyPose = headMesh->modelMatrix()*glm::vec4(0.f, 0.f, 0.f, 1.f);
            monkeyPose = monkeyPose/monkeyPose.w;
            float dist = utils::eulerDistIgnoreW(monkeyPose,  glm::vec4(playerPos, 1.f));
            
            if (dist < 1.0f) { 
                std::cout<<"COLLIDE!!"<<std::endl;
                int32_t tileX = static_cast<int32_t>(floor((playerPos.z - offsetBoard.z) / utils::TILE_LENGTH_X));
                int32_t tileY = static_cast<int32_t>(floor((playerPos.x - offsetBoard.x) / utils::TILE_LENGTH_Z));
                std::cout << tileX << " " << tileY << std::endl;
                MemoryManager::removeEl(headMesh);
                dir = 100 + tileY * 10 + tileX;
                signalChange();

                headCount.headsCollected++;
            }
        }
        
        std::chrono::time_point curr_frame = timer.now();
        bezierCurveManager.timeStep(curr_frame);
        makeCameraMoves(playerPos, curr_frame);
        float delta = std::chrono::duration<float>(curr_frame - millisec_since_epoch).count();
        int rem = static_cast<int>(floor(delta / 0.4f));
        float pos = delta - 0.4f*rem;
        if(motion){
            if(!prev_motion) start_motion = curr_frame;
            if(pos > 0.2f){
                pos = pos - 0.2f;
                pos = 15.f*pos/0.2f;
                player->mesh = &monkeyPoses[15 - static_cast<int>(floor(pos))];
            }else{
                pos = 15.f*pos/0.2f;
                player->mesh = &monkeyPoses[static_cast<int>(floor(pos))];
                
            }
            prev_motion = motion;
            motion = false;
        }else{
            player->mesh = &monkeyPoses[0];
        }

        if(fabs(playerPos.z - prev_pos.z) >= 2.0f * utils::TILE_LENGTH_Z){
            if(playerPos.z < prev_pos.z){
                dir = 4;
                b->shiftLeft(lightManager, particleEmitterManager);
                MemoryManager::removeEl(boardRoot);
                boardRoot = new MeshTree("boardroot", std::nullopt);
                MemoryManager::addEl(boardRoot);
                scene.root->addChild(boardRoot->shared_from_this());
                offsetBoard.z -= 2.0f * utils::TILE_LENGTH_Z;
                boardRoot->transform.translate = offsetBoard;
                signalChange();
                b->load(boardCopy, InitialState, 0UL, 7UL, 0UL, 2UL);
                for(int i = 0; i < 7; i ++){
                    for(int j = 0; j < 7; j++){
                        
                        std::cout<<b->board[i][j]->tag<<"\t";
                        boardRoot->addChild(b->board[i][j]->shared_from_this());
                    }
                    std::cout<<std::endl;
                }

                for(int i = 0; i < 7; i ++){
                    for(int j = 0; j < 7; j++){
                        std::cout<<boardCopy[i][j]->tileType<<"\t";
                        
                    }
                    std::cout<<std::endl;
                }
                
            }else{
                dir = 2;
                b->shiftRight(lightManager, particleEmitterManager);
                MemoryManager::removeEl(boardRoot);
            
                boardRoot = new MeshTree("boardroot", std::nullopt);
                MemoryManager::addEl(boardRoot);
                scene.root->addChild(boardRoot->shared_from_this());
                offsetBoard.z += 2.0f * utils::TILE_LENGTH_Z;
                boardRoot->transform.translate = offsetBoard;
                signalChange();
                b->load(boardCopy, InitialState, 0UL, 7UL, 5UL, 7UL);
                for(int i = 0; i < 7; i ++){
                    for(int j = 0; j < 7; j++){
                        std::cout<<boardCopy[i][j]->tileType<<"\t";
                        boardRoot->addChild(b->board[i][j]->shared_from_this());
                    }
                    std::cout<<std::endl;
                }
               

            }
            prev_pos.z = playerPos.z;
        }

        if(fabs(playerPos.x - prev_pos.x) >= 2.0f * utils::TILE_LENGTH_X){
            if(playerPos.x > prev_pos.x){
                std::cout<<"down"<<std::endl;
                dir = 3;
                b->shiftDown(lightManager, particleEmitterManager);
                MemoryManager::removeEl(boardRoot);
            
                boardRoot = new MeshTree("boardroot", std::nullopt);
                MemoryManager::addEl(boardRoot);
                scene.root->addChild(boardRoot->shared_from_this());
                offsetBoard.x += 2.0f * utils::TILE_LENGTH_X;
                boardRoot->transform.translate = offsetBoard;
                signalChange();
                b->load(boardCopy, InitialState, 5UL, 7UL, 0UL, 7UL);
                for(int i = 0; i < 7; i ++){
                    for(int j = 0; j < 7; j++){
                        
                        std::cout<<b->board[i][j]->tag<<"\t";
                        boardRoot->addChild(b->board[i][j]->shared_from_this());
                    }
                    std::cout<<std::endl;
                }

                for(int i = 0; i < 7; i ++){
                    for(int j = 0; j < 7; j++){
                        std::cout<<boardCopy[i][j]->tileType<<"\t";
                        
                    }
                    std::cout<<std::endl;
                }
                
            }else{
                dir = 1;
                b->shiftUp(lightManager, particleEmitterManager);
                MemoryManager::removeEl(boardRoot);
            
                boardRoot = new MeshTree("boardroot", std::nullopt);
                MemoryManager::addEl(boardRoot);
                scene.root->addChild(boardRoot->shared_from_this());
                offsetBoard.x -= 2.0f * utils::TILE_LENGTH_X;
                boardRoot->transform.translate = offsetBoard;
                signalChange();
                b->load(boardCopy, InitialState, 0UL, 2UL, 0UL, 7UL);
                for(int i = 0; i < 7; i ++){
                    for(int j = 0; j < 7; j++){
                        std::cout<<boardCopy[i][j]->tileType<<"\t";
                        boardRoot->addChild(b->board[i][j]->shared_from_this());
                    }
                    std::cout<<std::endl;
                }
                
            }
            prev_pos.x = playerPos.x;
        }

        // View-projection matrices setup
        const float fovRadiansMain              = glm::radians(cameraZoomed ? renderConfig.zoomedVerticalFOV : renderConfig.verticalFOV);
        const glm::mat4 viewProjectionMain      = glm::perspective(fovRadiansMain, utils::ASPECT_RATIO, 0.1f, 30.0f) * currentCamera.viewMatrix();

        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        
        // Update transformation of managed mesh tree objects
        scene.root->transformExternal();
        
        // Particle simulation
        particleEmitterManager.updateEmitters();
        
        // Render shadow maps
        utils::renderShadowMaps(scene.root, renderConfig, lightManager);
        
        // Render scene and 3D GUI objects
        mainRenderer.render(viewProjectionMain, currentCamera.cameraPos());
        mainRenderer.copyGBufferDepth(0); // Copy main renderer G-buffer depth data to main framebuffer so 3D UI elements are depth tested appropriately
        menu.draw3D(viewProjectionMain);
        
        // Draw minimap if desired
        if (renderConfig.drawMinimap ) {// && !xToonPowerUp) {
            const float fovRadiansMinimap           = glm::radians(renderConfig.minimapVerticalFOV);

            const glm::vec3 position = playerPos + glm::vec3(0.0f, 10.0f, 0.0f);
            const glm::mat4 viewMatrix = glm::lookAt(position,
                                                     playerPos,
                                                     glm::vec3(0.0f, 0.0f, -1.0f));

            const glm::mat4 viewProjectionMinimap = glm::perspective(fovRadiansMinimap, utils::ASPECT_RATIO, 0.1f, 30.0f) * viewMatrix;
            minimapRenderer.render(viewProjectionMinimap, position);
        }

        // Draw 2D GUI
        menu.draw2D();

        // Process inputs and swap the window buffer
        m_window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
