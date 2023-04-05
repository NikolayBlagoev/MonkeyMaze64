#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
#include <GLFW/glfw3.h> // Include glad before glfw3
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()
#include <framework/window.h>

#include <gameplay/enemy_camera.h>
#include <generator/board.hpp>
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
bool cameraZoomed = false;
std::mutex m;
std::condition_variable cv;
bool ready_ = false;
bool processed_ = false;
std::vector<std::weak_ptr<EnemyCamera>> cameras;
std::vector<std::weak_ptr<MeshTree>> monkeyHeads;
Defined*** boardCopy;
const float factorx = 7.72f;
const float factory = 7.72f;
const float board_init_off = -6.2f;
bool motion = false;
glm::vec3 offsetBoard ( -3*factorx,0.f,-3*factory);
int dir = -1;
bool* ready = new bool;
bool processed = false;
Generator* gen =  new Generator();

void signalChange(){
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

void worker_thread()
{
    
    gen->instantiate_terr();
    boardCopy = gen->board;
    gen->visualise(gen->board, 7,7);
    while(true){
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
            gen->assign_all(&(gen->dq));
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

void makeCameraMoves(){
    for(size_t i = 0; i < cameras.size(); i++){
        if(cameras.at(i).expired()){
            cameras.erase(cameras.begin() + i);
            i--;
            continue;
        }
        std::shared_ptr<EnemyCamera> cam = cameras.at(i).lock();
        // IF PLAYER IN VIEW DO STUFF
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
}

int main(int argc, char* argv[]) {
    *ready = false;
    std::thread worker(worker_thread);

    // Player position data
    glm::vec3 playerPos = glm::vec3(0.0f, -0.1f, 1.0f);
    glm::vec3 playerMiddleOffset = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 playerCameraPos = playerPos + glm::vec3(0.0f, 1.0f, 1.5f) + playerMiddleOffset;
    glm::vec3 playerLightOffset = glm::vec3(-0.9f, 2.5f, 1.3f);

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
    DeferredRenderer deferredRenderer(renderConfig, scene, lightManager, particleEmitterManager, xToonTex);
    Menu menu(scene, renderConfig, lightManager, particleEmitterManager, deferredRenderer);

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

    // Load object meshes
    Mesh camera     = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "camera.obj"));
    Mesh aperture   = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "aperture.obj"));
    Mesh stand2     = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "stand2.obj"));
    Mesh stand1     = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "stand1.obj"));
    Mesh suzanne    = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "suzanne.obj"));

    // Load player character animation models and set textures
    Mesh defaultMonkeyPose = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose0.obj"));
    std::vector<GPUMesh> monkeyPoses;
    monkeyPoses.emplace_back(defaultMonkeyPose);
    for (int i = 1; i < 16; ++i) {
        // Load model and create mesh object
        auto fileName   = "monkeypose" + std::to_string(i * 2)+ ".obj";
        Mesh cpuMesh    = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / fileName));
        monkeyPoses.emplace_back(cpuMesh);

        // Link textures
        monkeyPoses[i].setAlbedo(albedoFur);
        monkeyPoses[i].setAO(aoFur);
        monkeyPoses[i].setDisplacement(displacementFur, true);
        monkeyPoses[i].setNormal(normalFur);
        monkeyPoses[i].setRoughness(roughnessFur);
    }

    // Add player mesh node
    MeshTree* player = new MeshTree("player", &defaultMonkeyPose, playerPos,
                                    glm::vec4(0.0f, 1.0f, 0.0f, 180.0f),
                                    glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                                    glm::vec3(0.3f),
                                    true);
    MemoryManager::addEl(player);
    scene.addMesh(player->shared_from_this());

    // Add player torch mesh node
    MeshTree* playerLight = new MeshTree("player light");
    MemoryManager::addEl(playerLight);
    playerLight->transform.translate = playerLightOffset;
    playerLight->pl = lightManager.addPointLight(glm::vec3(0.f), glm::vec3(1.0f, 0.5f, 0.0f), 0.3f);
    player->addChild(playerLight->shared_from_this());

    // Load tile meshes
    Mesh crossing   = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "crossing.obj"));
    Mesh room       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "room.obj"));
    Mesh tjunction  = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tjunction.obj"));
    Mesh tunnel     = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tunnel.obj"));
    Mesh turn       = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "turn.obj"));

    // Add test lights
    lightManager.addPointLight(glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 3.0f);
    lightManager.addPointLight(glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 3.0f);
    lightManager.addPointLight(glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), 3.0f);
    lightManager.addAreaLight(glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
    lightManager.addAreaLight(glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(0.0f, 0.5f, 0.0f));

    // Init MeshTree root
    MeshTree* boardRoot = new MeshTree("board root");
    MemoryManager::addEl(boardRoot);
    boardRoot->transform.translate = offsetBoard;
   
    // Init Bezier curves
    std::chrono::high_resolution_clock timer; 
    BezierCurve<glm::vec3> b3d                      = BezierCurve<glm::vec3>(glm::vec3(0.f), glm::vec3(1.f , 1.f, 0.f), glm::vec3(-1.f , 2.f, 0.f), glm::vec3(0.f, 3.f, 0.f), 10.f);
    BezierCurve<glm::vec3> b3d2                     = BezierCurve<glm::vec3>(glm::vec3(0.f, 3.f, 0.f), glm::vec3(-1.f , 2.f, 0.f), glm::vec3(1.f , 1.f, 0.f), glm::vec3(0.f), 10.f);
    BezierCurve<glm::vec4> b4d                      = BezierCurve<glm::vec4>(glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(0.f , 0.3826834f, 0.f, 0.9238795f), glm::vec4(0.f , 0.7132504f, 0.f, 0.7009093f), glm::vec4(0.f , 1.f, 0.f, 0.f), 10.f);
    BezierComposite<glm::vec3> b3c                  = BezierComposite<glm::vec3>({b3d, b3d2}, true, 20.f);
    std::chrono::time_point millisec_since_epoch    = timer.now();
    BezierCurveManager rndrr                        = BezierCurveManager(millisec_since_epoch);

    bool prev_motion                        = motion;
    std::chrono::time_point start_motion    = millisec_since_epoch;
    glm::vec3 prev_pos                      = playerPos;

    // Create root node of board
    boardRoot = new MeshTree("boardoot");
    MemoryManager::addEl(boardRoot);
    scene.root->addChild(boardRoot->shared_from_this());
    
    boardRoot->transform.translate = offsetBoard;
    b = new Board(boardCopy, {&crossing, &room, &tjunction, &tunnel, &turn, &camera, &aperture, &stand2, &stand1, &suzanne, rndrr, lightManager, cameras, monkeyHeads});
    for(int i = 0; i < 7; i ++){
        for(int j = 0; j < 7; j++)
            boardRoot->addChild(b->board[i][j]->shared_from_this());
    }

    // Main loop
    while (!m_window.shouldClose()) {
        playerPos = (player->transform.translate);
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
                int32_t tileX = static_cast<int32_t>(floor((playerPos.z - offsetBoard.z) / factorx));
                int32_t tileY = static_cast<int32_t>(floor((playerPos.x - offsetBoard.x) / factory));
                std::cout << tileX << " " << tileY << std::endl;
                MemoryManager::removeEl(headMesh);
                dir = 100 + tileY * 10 + tileX;
                signalChange();
            }
        }
        Camera& currentCamera = renderConfig.controlPlayer ? playerCamera : mainCamera;
        std::chrono::time_point curr_frame = timer.now();
        rndrr.timeStep(curr_frame);
        makeCameraMoves();
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

        // Clear the screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        if(fabs(playerPos.z - prev_pos.z) >= 2*factory){
            if(playerPos.z < prev_pos.z){
                dir = 4;
                b->move_l(lightManager);
                MemoryManager::removeEl(boardRoot);
            
                boardRoot = new MeshTree("boardroot");
                MemoryManager::addEl(boardRoot);
                scene.root->addChild(boardRoot->shared_from_this());
                offsetBoard.z -=2*factory;
                boardRoot->transform.translate = offsetBoard;
                signalChange();
                b->load(boardCopy, {&crossing, &room, &tjunction, &tunnel, &turn, &camera, &aperture, &stand2, &stand1, &suzanne, rndrr, lightManager, cameras, monkeyHeads},
                0, 7, 0, 2);
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
                prev_pos = playerPos;
            }else{
                dir = 2;
                b->move_r(lightManager);
                MemoryManager::removeEl(boardRoot);
            
                boardRoot = new MeshTree("boardroot");
                MemoryManager::addEl(boardRoot);
                scene.root->addChild(boardRoot->shared_from_this());
                offsetBoard.z +=2*factory;
                boardRoot->transform.translate = offsetBoard;
                signalChange();
                b->load(boardCopy, {&crossing, &room, &tjunction, &tunnel, &turn, &camera, &aperture, &stand2, &stand1, &suzanne, rndrr, lightManager, cameras, monkeyHeads},
                0, 7, 5, 7);
                for(int i = 0; i < 7; i ++){
                    for(int j = 0; j < 7; j++){
                        std::cout<<boardCopy[i][j]->tileType<<"\t";
                        boardRoot->addChild(b->board[i][j]->shared_from_this());
                    }
                    std::cout<<std::endl;
                }
                prev_pos = playerPos;

            }
        }

        if(fabs(playerPos.x - prev_pos.x) >= 2*factorx){
            if(playerPos.x > prev_pos.x){
                std::cout<<"down"<<std::endl;
                dir = 3;
                b->move_d(lightManager);
                MemoryManager::removeEl(boardRoot);
            
                boardRoot = new MeshTree("boardroot");
                MemoryManager::addEl(boardRoot);
                scene.root->addChild(boardRoot->shared_from_this());
                offsetBoard.x += 2*factorx;
                boardRoot->transform.translate = offsetBoard;
                signalChange();
                b->load(boardCopy, {&crossing, &room, &tjunction, &tunnel, &turn, &camera, &aperture, &stand2, &stand1, &suzanne, rndrr, lightManager, cameras, monkeyHeads},
                5, 7, 0, 7);
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
                prev_pos = playerPos;
            }else{
                dir = 1;
                b->move_u(lightManager);
                MemoryManager::removeEl(boardRoot);
            
                boardRoot = new MeshTree("boardroot");
                MemoryManager::addEl(boardRoot);
                scene.root->addChild(boardRoot->shared_from_this());
                offsetBoard.x -=2*factorx;
                boardRoot->transform.translate = offsetBoard;
                signalChange();
                b->load(boardCopy, {&crossing, &room, &tjunction, &tunnel, &turn, &camera, &aperture, &stand2, &stand1, &suzanne, rndrr, lightManager, cameras, monkeyHeads},
                0, 2, 0, 7);
                for(int i = 0; i < 7; i ++){
                    for(int j = 0; j < 7; j++){
                        std::cout<<boardCopy[i][j]->tileType<<"\t";
                        boardRoot->addChild(b->board[i][j]->shared_from_this());
                    }
                    std::cout<<std::endl;
                }
                prev_pos = playerPos;

            }
        }

        // Controls
        ImGuiIO io = ImGui::GetIO();
        m_window.updateInput();
        if (!io.WantCaptureMouse) { // Prevent camera movement when accessing UI elements
            if (renderConfig.controlPlayer) { currentCamera.updateInput(player, scene.root, playerMiddleOffset); }
            else                            { currentCamera.updateInput(); }
        }

        // View and projection matrices setup
        const float fovRadians              = glm::radians(cameraZoomed ? renderConfig.zoomedVerticalFOV : renderConfig.verticalFOV);
        const glm::mat4 m_viewProjection    = glm::perspective(fovRadians, utils::ASPECT_RATIO, 0.1f, 30.0f) * currentCamera.viewMatrix();

        // Particle simulation
        particleEmitterManager.updateEmitters();

        // Render shadow maps
        utils::renderShadowMaps(scene.root, renderConfig, lightManager);

        // Render scene
        deferredRenderer.render(m_viewProjection, currentCamera.cameraPos());

        // Draw UI
        glViewport(0, 0, utils::WIDTH, utils::HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        menu.draw(m_viewProjection); // Assume identity model matrix

        // Process inputs and swap the window buffer
        m_window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
