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

void onMouseClicked(int , int ) {
    // signalChange();
}

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

    // Load textures
    std::weak_ptr<const Texture> rustAlbedo     = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_basecolor.png");
    std::weak_ptr<const Texture> rustNormal     = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_normal.png");
    std::weak_ptr<const Texture> rustMetallic   = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_metallic.png");
    std::weak_ptr<const Texture> rustRoughness  = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_roughness.png");
    std::weak_ptr<const Texture> stoneAlbedo    = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_basecolor.png");
    std::weak_ptr<const Texture> stoneNormal    = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_basecolor.png");
    std::weak_ptr<const Texture> stoneRoughness = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_basecolor.png");
    std::weak_ptr<const Texture> stoneAO        = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_basecolor.png");

    // Load meshes
    Mesh camera = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "camera.obj"));
    Mesh aperture = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "aperture.obj"));
    Mesh stand2 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "stand2.obj"));
    Mesh stand1 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "stand1.obj"));
    Mesh suzanne = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "suzanne.obj"));
    Mesh dragonMesh = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "dragon.obj"));
    Mesh monkeypose0 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose0.obj"));
    Mesh monkeypose2 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose2.obj"));
    Mesh monkeypose4 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose4.obj"));
    Mesh monkeypose6 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose6.obj"));
    Mesh monkeypose8 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose8.obj"));
    Mesh monkeypose10 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose10.obj"));
    Mesh monkeypose12 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose12.obj"));
    Mesh monkeypose14 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose14.obj"));
    Mesh monkeypose16 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose16.obj"));
    Mesh monkeypose18 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose18.obj"));
    Mesh monkeypose20 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose20.obj"));
    Mesh monkeypose22 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose22.obj"));
    Mesh monkeypose24 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose24.obj"));
    Mesh monkeypose26 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose26.obj"));
    Mesh monkeypose28 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose28.obj"));
    Mesh monkeypose30 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "animated" / "monkeypose30.obj"));
    GPUMesh monkeyposes[16] = {GPUMesh(monkeypose0), GPUMesh(monkeypose2), GPUMesh(monkeypose4), GPUMesh(monkeypose6),
                        GPUMesh(monkeypose8), GPUMesh(monkeypose10), GPUMesh(monkeypose12), GPUMesh(monkeypose14),
                        GPUMesh(monkeypose16), GPUMesh(monkeypose18), GPUMesh(monkeypose20), GPUMesh(monkeypose22),
                        GPUMesh(monkeypose24), GPUMesh(monkeypose26), GPUMesh(monkeypose28), GPUMesh(monkeypose30)};

    MeshTree* bezierDragon = new MeshTree("bezier dragon", &dragonMesh);
    MemoryManager::addEl(bezierDragon);
    scene.addMesh(bezierDragon->shared_from_this());

    MeshTree* playerDragon = new MeshTree("player", &monkeypose0, playerPos,
                                          glm::vec4(0.0f, 1.0f, 0.0f, 180.0f),
                                          glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                                          glm::vec3(0.3f),
                                          true);
    MemoryManager::addEl(playerDragon);
    scene.addMesh(playerDragon->shared_from_this());
    MeshTree* tempMesh = new MeshTree("dragon particle", &dragonMesh, playerPos + glm::vec3(1.0f, 0.0f, 0.0f),
                                   glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                                   glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                                   glm::vec3(1.0f),
                                   false);
    MemoryManager::addEl(tempMesh);
    scene.addMesh(tempMesh->shared_from_this());

    Mesh crossing = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "crossing.obj"));
    Mesh room = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "room.obj"));
    Mesh tjunction = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tjunction.obj"));
    Mesh tunnel = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "tunnel.obj"));
    Mesh turn = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "turn.obj"));

    // Add test lights
    lightManager.addPointLight(glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 3.0f);
    lightManager.addPointLight(glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 3.0f);
    lightManager.addPointLight(glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), 3.0f);
    lightManager.addAreaLight(glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
    lightManager.addAreaLight(glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(0.0f, 0.5f, 0.0f));

    // Add particle emitter(s)
    particleEmitterManager.addEmitter({ 1.0f, 1.0f, 1.0f });

    // Init MeshTree
    MeshTree* boardRoot = new MeshTree("board root");
    MemoryManager::addEl(boardRoot);
    boardRoot->transform.translate = offsetBoard;
   
    // Init Bezier curves
    std::chrono::high_resolution_clock timer; 
    BezierCurve3d b3d                               = BezierCurve3d(glm::vec3(0.f), glm::vec3(1.f , 1.f, 0.f), glm::vec3(-1.f , 2.f, 0.f), glm::vec3(0.f, 3.f, 0.f), 10.f);
    BezierCurve3d b3d2                              = BezierCurve3d(glm::vec3(0.f, 3.f, 0.f), glm::vec3(-1.f , 2.f, 0.f), glm::vec3(1.f , 1.f, 0.f), glm::vec3(0.f), 10.f);
    BezierCurve4d b4d                               = BezierCurve4d(glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(0.f , 0.3826834f, 0.f, 0.9238795f), glm::vec4(0.f , 0.7132504f, 0.f, 0.7009093f), glm::vec4(0.f , 1.f, 0.f, 0.f), 10.f);
    CompositeBezier3d b3c                           = CompositeBezier3d({&b3d,&b3d2}, true, 20.f);
    std::chrono::time_point millisec_since_epoch    = timer.now();
    BezierCurveRenderer rndrr                       = BezierCurveRenderer(millisec_since_epoch);
    b3c.start_time                                  = millisec_since_epoch;
    b4d.prev_time                                   = millisec_since_epoch;

    bool prev_motion = motion;
    std::chrono::time_point start_motion = millisec_since_epoch;
    glm::vec3 prev_pos = playerPos;

    boardRoot = new MeshTree("boardoot");
    MemoryManager::addEl(boardRoot);
    scene.root->addChild(boardRoot->shared_from_this());
    
    boardRoot->transform.translate = offsetBoard;
    b = new Board(boardCopy, {&crossing, &room, &tjunction, &tunnel, &turn, &camera, &aperture, &stand2, &stand1, &suzanne, rndrr, lightManager, cameras});
    for(int i = 0; i < 7; i ++){
        for(int j = 0; j < 7; j++)
            boardRoot->addChild(b->board[i][j]->shared_from_this());
    }

    // Main loop
    while (!m_window.shouldClose()) {
        playerPos = (playerDragon->transform.translate);
        Camera& currentCamera = renderConfig.controlPlayer ? playerCamera : mainCamera;
        std::chrono::time_point curr_frame = timer.now();
        rndrr.do_moves(curr_frame);
        makeCameraMoves();
        float delta = std::chrono::duration<float>(curr_frame - millisec_since_epoch).count();
        int rem = static_cast<int>(floor(delta / 0.4f));
        float pos = delta - 0.4f*rem;
        if(motion){
            if(!prev_motion) start_motion = curr_frame;
            if(pos > 0.2f){
                pos = pos - 0.2f;
                pos = 15.f*pos/0.2f;
                playerDragon->mesh = &monkeyposes[15 - static_cast<int>(floor(pos))];
            }else{
                pos = 15.f*pos/0.2f;
                playerDragon->mesh = &monkeyposes[static_cast<int>(floor(pos))];
                
            }
            prev_motion = motion;
            motion = false;
        }else{
            playerDragon->mesh = &monkeyposes[0];
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
                b->load(boardCopy, {&crossing, &room, &tjunction, &tunnel, &turn, &camera, &aperture, &stand2, &stand1, &suzanne, rndrr, lightManager, cameras},
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
                b->load(boardCopy, {&crossing, &room, &tjunction, &tunnel, &turn, &camera, &aperture, &stand2, &stand1, &suzanne, rndrr, lightManager, cameras},
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
                b->load(boardCopy, {&crossing, &room, &tjunction, &tunnel, &turn, &camera, &aperture, &stand2, &stand1, &suzanne, rndrr, lightManager, cameras},
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
                b->load(boardCopy, {&crossing, &room, &tjunction, &tunnel, &turn, &camera, &aperture, &stand2, &stand1, &suzanne, rndrr, lightManager, cameras},
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
            if (renderConfig.controlPlayer) { currentCamera.updateInput(playerDragon, scene.root, playerMiddleOffset); }
            else                            { currentCamera.updateInput(); }
        }

        // View-projection matrices setup
        const float fovRadians = glm::radians(cameraZoomed ? renderConfig.zoomedVerticalFOV : renderConfig.verticalFOV);
        const glm::mat4 m_viewProjectionMatrix = glm::perspective(fovRadians, utils::ASPECT_RATIO, 0.1f, 30.0f) * currentCamera.viewMatrix();

        // Particle simulation
        particleEmitterManager.updateEmitters();

        // Render shadow maps
        utils::renderShadowMaps(scene.root, renderConfig, lightManager);

        // Render scene
        deferredRenderer.render(m_viewProjectionMatrix, currentCamera.cameraPos());

        // Draw UI
        glViewport(0, 0, utils::WIDTH, utils::HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        menu.draw(m_viewProjectionMatrix);

        // Process inputs and swap the window buffer
        m_window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
