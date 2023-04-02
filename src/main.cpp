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
#include "camera.h"
// Always include window first (because it includes glfw, which includes GL which needs to be included AFTER glew).
// Can't wait for modules to fix this stuff...
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
// Include glad before glfw3
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>
#include <framework/window.h>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <vector>
#include <render/bezier.h>
#include <ctime>
#include <chrono>
#include <generator/generator.h>

// Game state
// TODO: Have a separate struct for this if it becomes too much
bool cameraZoomed = false;
std::mutex m;
std::condition_variable cv;
bool ready_ = false;
std::vector<std::weak_ptr<CameraObj>> cameras;
Defined*** boardCopy;
const float factorx = 7.72f;
const float factory = 7.72f;
const float board_init_off = -6.2f;
bool motion = false;
glm::vec3 offsetBoard ( -3*factorx,0.f,-3*factory);

bool* ready = new bool;
bool processed = false;
Generator* gen =  new Generator();
void signalChange(){
    std::cout<<"Change\n";
    {
        ready_ = true;
        std::unique_lock<std::mutex> lk(m);
        
        cv.notify_all();
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
            std::cout<<"WE GOOOD\n";
            ready_ = false;
            lk.unlock();
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

void drawMeshTreePtL(MeshTree* mt, const PointLight& light, Shader& m_pointShadowShader,
                     const glm::mat4& pointLightShadowMapsProjection, RenderConfig& renderConfig){
    if(mt == nullptr) return;

    const glm::mat4& modelMatrix = mt->modelMatrix();
    if(mt->mesh != nullptr){
        const GPUMesh& mesh                         = *(mt->mesh);
        // std::cout<<"DRAWING"<<std::endl;
        const std::array<glm::mat4, 6U> lightMvps   = light.genMvpMatrices(modelMatrix, pointLightShadowMapsProjection);

         // Render each cubemap face
        for (size_t face = 0UL; face < 6UL; face++) {
            // Bind shadow shader and shadowmap framebuffer
            m_pointShadowShader.bind();
            glBindFramebuffer(GL_FRAMEBUFFER, light.framebuffers[face]);
            glEnable(GL_DEPTH_TEST);

            // Bind uniforms
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(lightMvps[face]));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniform3fv(2, 1, glm::value_ptr(light.position));
            glUniform1f(3, renderConfig.shadowFarPlane);

            // Bind model's VAO and draw its elements
            mesh.draw();
        }
    }
    for (size_t i = 0; i < mt->children.size(); i++) {
        std::weak_ptr<MeshTree> tmp = mt->children.at(i);
        if(tmp.expired()){
            mt->children.erase(mt->children.begin()+i);
            i--;
            continue;
        }
        std::shared_ptr<MeshTree> shrtmp = tmp.lock();
        drawMeshTreePtL(shrtmp.get(), light, m_pointShadowShader, pointLightShadowMapsProjection, renderConfig);
    }
}

void drawMeshTreeAL(MeshTree* mt, const glm::mat4& areaLightShadowMapsProjection, const glm::mat4& lightView, RenderConfig& renderConfig){
    if(mt == nullptr) return;
    
    const glm::mat4& modelMatrix = mt->modelMatrix();
    if(mt->mesh != nullptr){
        const GPUMesh& mesh                         = *(mt->mesh);
    
             

        // Bind light camera mvp matrix
        const glm::mat4 lightMvp = areaLightShadowMapsProjection * lightView *  modelMatrix;
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(lightMvp));

        // Bind model's VAO and draw its elements
        mesh.draw();
    }
    for (size_t i = 0; i < mt->children.size(); i++) {
        std::weak_ptr<MeshTree> tmp = mt->children.at(i);
        if(tmp.expired()){
            mt->children.erase(mt->children.begin()+i);
            i--;
            continue;
        }
        std::shared_ptr<MeshTree> shrtmp = tmp.lock();
        drawMeshTreeAL(shrtmp.get(), areaLightShadowMapsProjection, lightView, renderConfig);
    }
        
 
}
void makeCameraMoves(){
    for(size_t i = 0; i < cameras.size(); i++){
        if(cameras.at(i).expired()){
            cameras.erase(cameras.begin() + i);
            i--;
            continue;
        }
        std::shared_ptr<CameraObj> cam = cameras.at(i).lock();
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
// CameraObj* makeCamera(Mesh* aperture, Mesh* camera, Mesh* stand2, Mesh* stand1,
//                       glm::vec3 tr, glm::vec4 selfRot, glm::vec4 parRot, glm::vec3 scl){
    
//     return cam;
// }

void addObjectsRoom(MeshTree* room, Defined* roomTile, Mesh* aperture, Mesh* camera, Mesh* stand2, Mesh* stand1, Mesh* suzanne, BezierCurveRenderer& rndrr){
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

int main() {
    // Init core objects
    *ready = false;
    std::thread worker(worker_thread);

    glm::vec3 playerPos = glm::vec3(0.0f, -0.1f, 1.0f);
    glm::vec3 playerCameraPos = playerPos + glm::vec3(0.0f, 1.5f, 1.5f);

    RenderConfig renderConfig;
    Window m_window("Final Project", glm::ivec2(utils::WIDTH, utils::HEIGHT), OpenGLVersion::GL46);
    Camera mainCamera(&m_window, renderConfig, glm::vec3(2.0f, 3.0f, 0.0f), -glm::vec3(1.0f, 1.1f, 0.0f));
    Camera playerCamera(&m_window, renderConfig, playerCameraPos, (playerPos - playerCameraPos) + glm::vec3(0.f,1.f,0.f));
    playerCamera.update = &motion;
    TextureManager textureManager;
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

    // Build shadow shaders
    Shader m_pointShadowShader;
    Shader m_areaShadowShader;
    try {
        ShaderBuilder pointShadowBuilder;
        pointShadowBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "shadows" / "shadow_point.vert");
        pointShadowBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "shadows" / "shadow_point.frag");
        m_pointShadowShader = pointShadowBuilder.build();

        ShaderBuilder areaShadowBuilder;
        areaShadowBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "shadows" / "shadow_stock.vert");
        m_areaShadowShader = areaShadowBuilder.build();
    } catch (ShaderLoadingException e) { std::cerr << e.what() << std::endl; }

    // Load textures
    std::weak_ptr<const Texture> rustAlbedo     = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_basecolor.png");
    std::weak_ptr<const Texture> rustNormal     = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_normal.png");
    std::weak_ptr<const Texture> rustMetallic   = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_metallic.png");
    std::weak_ptr<const Texture> rustRoughness  = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_roughness.png");
    std::weak_ptr<const Texture> stoneAlbedo    = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_basecolor.png");
    std::weak_ptr<const Texture> stoneNormal    = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_basecolor.png");
    std::weak_ptr<const Texture> stoneRoughness = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_basecolor.png");
    std::weak_ptr<const Texture> stoneAO        = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "rustediron2_basecolor.png");

    Mesh camera = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "camera.obj"));
    Mesh aperture = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "aperture.obj"));
    Mesh stand2 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "stand2.obj"));
    Mesh stand1 = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "stand1.obj"));

    Mesh suzanne = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "suzanne.obj"));
    // Add models
    Mesh dragonMesh = mergeMeshes(loadMesh(utils::RESOURCES_DIR_PATH / "models" / "dragon.obj"));
    // scene.addMesh(utils::RESOURCES_DIR_PATH / "models" / "dragonWithFloor.obj");
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
    MeshTree* bezierDragon = new MeshTree(&dragonMesh);
    scene.addMesh(bezierDragon->self);

    MeshTree* playerDragon = new MeshTree(&monkeypose0, playerPos,
                                          glm::vec4(0.0f, 1.0f, 0.0f, 180.0f),
                                          glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                                          glm::vec3(0.3f),
                                          true);
    scene.addMesh(playerDragon->self);
    MeshTree* tempMesh = new MeshTree(&dragonMesh, playerPos + glm::vec3(1.0f, 0.0f, 0.0f),
                                   glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                                   glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
                                   glm::vec3(1.0f),
                                   false);
    scene.addMesh(tempMesh->self);

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
    MeshTree* boardRoot = new MeshTree();
    boardRoot->transform.translate = offsetBoard;
   
    std::chrono::high_resolution_clock timer; 
    BezierCurve3d b3d = BezierCurve3d(glm::vec3(0.f), glm::vec3(1.f , 1.f, 0.f), glm::vec3(-1.f , 2.f, 0.f), glm::vec3(0.f, 3.f, 0.f), 10.f);
    BezierCurve3d b3d2 = BezierCurve3d(glm::vec3(0.f, 3.f, 0.f), glm::vec3(-1.f , 2.f, 0.f), glm::vec3(1.f , 1.f, 0.f), glm::vec3(0.f), 10.f);
    BezierCurve4d b4d = BezierCurve4d(glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(0.f , 0.3826834f, 0.f, 0.9238795f), glm::vec4(0.f , 0.7132504f, 0.f, 0.7009093f), glm::vec4(0.f , 1.f, 0.f, 0.f), 10.f);
    CompositeBezier3d b3c = CompositeBezier3d({&b3d,&b3d2}, true, 20.f);
    std::chrono::time_point millisec_since_epoch = timer.now();
    BezierCurveRenderer rndrr = BezierCurveRenderer(millisec_since_epoch);
    b3c.start_time = millisec_since_epoch;
    b4d.prev_time = millisec_since_epoch;
    BezierCombo3dcomp combo1 = BezierCombo3dcomp(&b3c, bezierDragon->translate);
    rndrr.add3dcomp(&combo1);
    BezierCombo4d combo2 = BezierCombo4d(&b4d, bezierDragon->selfRotate);
    rndrr.add4d(&combo2);
    bool flag = true;
    bool prev_motion = motion;
    std::chrono::time_point start_motion = millisec_since_epoch;

    // Main loop
    while (!m_window.shouldClose()) {
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
        if (flag){
            flag = false;
            free(boardRoot);
            
            boardRoot = new MeshTree();
            boardRoot->transform.translate = offsetBoard;
            for(int i = 0; i < 7; i ++){
                // glm::vec3 offset(0.f,10.f*i,0.f);
                for(int j = 0; j < 7; j++){
              
                    if(boardCopy[i][j]->tileType == 1 ){
                        MeshTree* crossingTile = new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        boardRoot->addChild(crossingTile->self);
                    }else if(boardCopy[i][j]->tileType == 2 ){
                        MeshTree* roomTile = new MeshTree(&room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        boardRoot->addChild(roomTile->self);
                        addObjectsRoom(roomTile, boardCopy[i][j], &aperture, &camera, &stand2, &stand1, &suzanne, rndrr);
                    }else if(boardCopy[i][j]->tileType == 3){
                        MeshTree* roomTile = new MeshTree(&room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        boardRoot->addChild(roomTile->self);
                        addObjectsRoom(roomTile, boardCopy[i][j], &aperture, &camera, &stand2, &stand1, &suzanne, rndrr);
                    }else if(boardCopy[i][j]->tileType == 4){
                        MeshTree* roomTile = new MeshTree(&room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        boardRoot->addChild(roomTile->self);
                        addObjectsRoom(roomTile, boardCopy[i][j], &aperture, &camera, &stand2, &stand1, &suzanne, rndrr);
                    }else if(boardCopy[i][j]->tileType == 5){
                        MeshTree* roomTile = new MeshTree(&room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        boardRoot->addChild(roomTile->self);
                        addObjectsRoom(roomTile, boardCopy[i][j], &aperture, &camera, &stand2, &stand1, &suzanne, rndrr);
                    }else if(boardCopy[i][j]->tileType == 6){
                        MeshTree* crossingTile = new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        boardRoot->addChild(crossingTile->self);
                    }else if(boardCopy[i][j]->tileType == 7){
                        boardRoot->addChild((new MeshTree(&tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)))->self);
                    }else if(boardCopy[i][j]->tileType == 8){
                        boardRoot->addChild((new MeshTree(&tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)))->self);
                    }else if(boardCopy[i][j]->tileType == 9){
                        boardRoot->addChild((new MeshTree(&tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)))->self);
                    }else if(boardCopy[i][j]->tileType == 10){
                        boardRoot->addChild((new MeshTree(&tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)))->self);
                    }else if(boardCopy[i][j]->tileType == 11){
                        boardRoot->addChild((new MeshTree(&turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)))->self);
                    }else if(boardCopy[i][j]->tileType == 12){
                        boardRoot->addChild((new MeshTree(&turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)))->self);
                    }else if(boardCopy[i][j]->tileType == 13){
                        boardRoot->addChild((new MeshTree(&turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f),  glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)))->self);
                    }else if(boardCopy[i][j]->tileType == 14){
                        boardRoot->addChild((new MeshTree(&turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)))->self);
                    }else if(boardCopy[i][j]->tileType == 15){
                        boardRoot->addChild((new MeshTree(&tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)))->self);
                    }else if(boardCopy[i][j]->tileType == 16){
                        boardRoot->addChild((new MeshTree(&tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)))->self);
                    }else if(boardCopy[i][j]->tileType == 17){
                        boardRoot->addChild((new MeshTree(&tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)))->self);
                    }else if(boardCopy[i][j]->tileType == 18){
                        boardRoot->addChild((new MeshTree(&tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)))->self);
                    } 
                }
            }
            scene.root->addChild(boardRoot->self);

        }

        // Controls
        ImGuiIO io = ImGui::GetIO();
        m_window.updateInput();
        if (!io.WantCaptureMouse) { // Prevent camera movement when accessing UI elements
            if (renderConfig.controlPlayer)
                currentCamera.updateInput(playerDragon, scene.root);
            else
                currentCamera.updateInput();
        }

        // View-projection matrices setup
        const float fovRadians = glm::radians(cameraZoomed ? renderConfig.zoomedVerticalFOV : renderConfig.verticalFOV);
        const glm::mat4 m_viewProjectionMatrix = glm::perspective(fovRadians, utils::ASPECT_RATIO, 0.1f, 30.0f) * currentCamera.viewMatrix();

        // Particle simulation
        particleEmitterManager.updateEmitters();

        // Render point lights shadow maps
        const glm::mat4 pointLightShadowMapsProjection = renderConfig.pointShadowMapsProjectionMatrix();
        glViewport(0, 0, utils::SHADOWTEX_WIDTH, utils::SHADOWTEX_HEIGHT); // Set viewport size to fit shadow map resolution

        for (size_t pointLightNum = 0U; pointLightNum < lightManager.numPointLights(); pointLightNum++) {
            const PointLight& light = lightManager.pointLightAt(pointLightNum);
            light.wipeFramebuffers();

            drawMeshTreePtL(scene.root, light, m_pointShadowShader, pointLightShadowMapsProjection, renderConfig);
        }

        // Render area lights shadow maps
        const glm::mat4 areaLightShadowMapsProjection = renderConfig.areaShadowMapsProjectionMatrix();
        glViewport(0, 0, utils::SHADOWTEX_WIDTH, utils::SHADOWTEX_HEIGHT); // Set viewport size to fit shadow map resolution
        for (size_t areaLightNum = 0U; areaLightNum < lightManager.numAreaLights(); areaLightNum++) {
            const AreaLight& light      = lightManager.areaLightAt(areaLightNum);
            const glm::mat4 lightView   = light.viewMatrix();

            // Bind shadow shader and shadowmap framebuffer
            m_areaShadowShader.bind();
            glBindFramebuffer(GL_FRAMEBUFFER, light.framebuffer);

            // Clear the shadow map and set needed options
            glClearDepth(1.0f);
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            // Render each model in the scene
            drawMeshTreeAL(scene.root, areaLightShadowMapsProjection, lightView, renderConfig);
       
        }

        // Render scene
        deferredRenderer.render(m_viewProjectionMatrix, currentCamera.cameraPos(), 0.f);

        // Draw UI
        glViewport(0, 0, utils::WIDTH, utils::HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        menu.draw(m_viewProjectionMatrix);

        // Process inputs and swap the window buffer
        m_window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
