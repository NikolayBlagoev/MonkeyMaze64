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
Defined*** boardCopy;
const float factorx = 7.72f;
const float factory = 7.72f;
const float board_init_off = -6.2f;
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

void drawMeshTreePtL(MeshTree* mt, const glm::mat4& currTransform, 
                     const PointLight& light, Shader& m_pointShadowShader, const glm::mat4& pointLightShadowMapsProjection, RenderConfig& renderConfig){
    if(mt == nullptr) return;

    const glm::mat4& modelMatrix = Scene::modelMatrix(mt, currTransform);
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
    for(MeshTree* child : mt->children){
        drawMeshTreePtL(child,modelMatrix, light, m_pointShadowShader, pointLightShadowMapsProjection, renderConfig);
    }
}

void drawMeshTreeAL(MeshTree* mt, const glm::mat4& currTransform, 
                    const glm::mat4& areaLightShadowMapsProjection, const glm::mat4& lightView, RenderConfig& renderConfig){
    if(mt == nullptr) return;
    
    const glm::mat4& modelMatrix = Scene::modelMatrix(mt, currTransform);
    if(mt->mesh != nullptr){
        const GPUMesh& mesh                         = *(mt->mesh);
    
             

        // Bind light camera mvp matrix
        const glm::mat4 lightMvp = areaLightShadowMapsProjection * lightView *  modelMatrix;
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(lightMvp));

        // Bind model's VAO and draw its elements
        mesh.draw();
    }
    for(MeshTree* child : mt->children){
        drawMeshTreeAL(child,modelMatrix, areaLightShadowMapsProjection, lightView, renderConfig);
    }
}

CameraObj* makeCamera(GPUMesh* aperture, GPUMesh* camera, GPUMesh* stand2, GPUMesh* stand1,
                      glm::vec3 tr, glm::vec4 selfRot, glm::vec4 parRot, glm::vec3 scl){
    MeshTree* ret = new MeshTree(stand1, tr, selfRot, parRot,  scl);
    MeshTree* stand2m = new MeshTree(stand2, glm::vec3(0.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(1.f));
    ret->addChild(stand2m);
    MeshTree* cameram = new MeshTree(camera, glm::vec3(0.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(1.f));
    stand2m->addChild(cameram);
    MeshTree* aperturem = new MeshTree(aperture, glm::vec3(0.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(1.f));
    cameram->addChild(aperturem);
    CameraObj* cam = new CameraObj(cameram, stand2m, ret);
    return cam;
}

void addObjectsRoom(MeshTree* room, Defined* roomTile, GPUMesh* aperture, GPUMesh* camera, GPUMesh* stand2, GPUMesh* stand1){
    for(int i = 0; i < roomTile->objs.size(); i++){
        if(roomTile->objs.at(i)->type == 0){
            CameraObj* cam = makeCamera(aperture, camera, stand2, stand1, glm::vec3(-9.9f, 9.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec3(1.f));
            room->addChild(cam->root);
        }
    }
}

int main() {
    // Init core objects
    *ready = false;
    std::thread worker(worker_thread);
    
    RenderConfig renderConfig;
    Window m_window("Final Project", glm::ivec2(utils::WIDTH, utils::HEIGHT), OpenGLVersion::GL46);
    Camera mainCamera(&m_window, renderConfig, 10.0f, glm::vec3(3.0f, 3.0f, 3.0f), -glm::vec3(1.2f, 1.1f, 0.9f));
    TextureManager textureManager;
    Scene scene;
    LightManager lightManager(renderConfig);
    ParticleEmitterManager particleEmitterManager(renderConfig);
    std::weak_ptr<const Texture> xToonTex = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "toon_map.png");
    DeferredRenderer mainRenderer(utils::WIDTH, utils::HEIGHT, 
                                  renderConfig, scene, lightManager, particleEmitterManager, xToonTex);
    DeferredRenderer minimapRenderer(utils::WIDTH / 2, utils::HEIGHT / 2, 
                                     renderConfig, scene, lightManager, particleEmitterManager, xToonTex);
    Menu menu(scene, renderConfig, lightManager, particleEmitterManager, mainRenderer);

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

    // Add models
    GPUMesh dragon = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "dragon.obj");
    scene.addMesh(utils::RESOURCES_DIR_PATH / "models" / "dragonWithFloor.obj");
    MeshTree* drg = new MeshTree(&dragon);
    scene.addMesh(drg);
    GPUMesh crossing = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "crossing.obj");
    GPUMesh room = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "room.obj");
    GPUMesh tjunction = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "tjunction.obj");
    GPUMesh tunnel = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "tunnel.obj");
    GPUMesh turn = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "turn.obj");

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
    boardRoot->offset = offsetBoard;
   
    std::chrono::high_resolution_clock timer; 
    BezierCurve3d b3d = BezierCurve3d(glm::vec3(0.f), glm::vec3(1.f , 1.f, 0.f), glm::vec3(-1.f , 2.f, 0.f), glm::vec3(0.f, 3.f, 0.f), 10.f);
    BezierCurve3d b3d2 = BezierCurve3d(glm::vec3(0.f, 3.f, 0.f), glm::vec3(-1.f , 2.f, 0.f), glm::vec3(1.f , 1.f, 0.f), glm::vec3(0.f), 10.f);
    BezierCurve4d b4d = BezierCurve4d(glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(0.f , 0.3826834f, 0.f, 0.9238795f), glm::vec4(0.f , 0.7132504f, 0.f, 0.7009093f), glm::vec4(0.f , 1.f, 0.f, 0.f), 10.f);
    CompositeBezier3d b3c = CompositeBezier3d({&b3d,&b3d2}, true, 20.f);
    std::chrono::time_point millisec_since_epoch = timer.now();
    BezierCurveRenderer rndrr = BezierCurveRenderer(millisec_since_epoch);
    b3c.start_time = millisec_since_epoch;
    b4d.prev_time = millisec_since_epoch;
    BezierCombo3dcomp combo1 = BezierCombo3dcomp(drg,&b3c, 0);
    rndrr.add3dcomp(&combo1);
    BezierCombo4d combo2 = BezierCombo4d(drg,&b4d, 0);
    rndrr.add4d(&combo2);
    bool flag = true;
    
    // Main loop
    while (!m_window.shouldClose()) {
        rndrr.do_moves(timer.now());
        float delta = std::chrono::duration<float>(timer.now() - millisec_since_epoch).count();
        float enred = delta/100.f;

        // Clear the screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        if (flag){
            flag = false;
            free(boardRoot);
            
            boardRoot = new MeshTree();
            boardRoot->offset = offsetBoard;
            for(int i = 0; i < 7; i ++){
                // glm::vec3 offset(0.f,10.f*i,0.f);
                for(int j = 0; j < 7; j++){
              
                    if(boardCopy[i][j]->tileType == 1 ){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 2 ){
                        MeshTree* roomTile = new MeshTree(&room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        boardRoot->addChild(roomTile);
                        // addObjectsRoom(roomTile, boardCopy[i][j], &aperture, &camera, &stand2, &stand1);
                    }else if(boardCopy[i][j]->tileType == 3){
                        MeshTree* roomTile = new MeshTree(&room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        boardRoot->addChild(roomTile);
                        // addObjectsRoom(roomTile, boardCopy[i][j], &aperture, &camera, &stand2, &stand1);
                    }else if(boardCopy[i][j]->tileType == 4){
                        MeshTree* roomTile = new MeshTree(&room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        boardRoot->addChild(roomTile);
                        // addObjectsRoom(roomTile, boardCopy[i][j], &aperture, &camera, &stand2, &stand1);
                    }else if(boardCopy[i][j]->tileType == 5){
                        MeshTree* roomTile = new MeshTree(&room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f));
                        boardRoot->addChild(roomTile);
                        // addObjectsRoom(roomTile, boardCopy[i][j], &aperture, &camera, &stand2, &stand1);
                    }else if(boardCopy[i][j]->tileType == 6){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 7){
                        boardRoot->addChild(new MeshTree(&tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 8){
                        boardRoot->addChild(new MeshTree(&tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 9){
                        boardRoot->addChild(new MeshTree(&tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 10){
                        boardRoot->addChild(new MeshTree(&tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 11){
                        boardRoot->addChild(new MeshTree(&turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 12){
                        boardRoot->addChild(new MeshTree(&turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 13){
                        boardRoot->addChild(new MeshTree(&turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f),  glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 14){
                        boardRoot->addChild(new MeshTree(&turn, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 15){
                        boardRoot->addChild(new MeshTree(&tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 270.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 16){
                        boardRoot->addChild(new MeshTree(&tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 17){
                        boardRoot->addChild(new MeshTree(&tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 90.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 18){
                        boardRoot->addChild(new MeshTree(&tjunction, glm::vec3(factorx*i, 0.f, factory*j), glm::vec4(0.f, 1.f, 0.f, 180.f), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec3(0.3f)));
                    } 
                }
            }
            scene.root->addChild(boardRoot);

        }
        
        // View-projection matrices setup
        const float fovRadians                  = glm::radians(cameraZoomed ? renderConfig.zoomedVerticalFOV : renderConfig.verticalFOV);
        const glm::mat4 viewProjectionMain      = glm::perspective(fovRadians, utils::ASPECT_RATIO, 0.1f, 30.0f) * mainCamera.viewMatrix();
        const glm::mat4 viewProjectionMinimap   = glm::perspective(fovRadians, utils::ASPECT_RATIO, 0.1f, 30.0f) * mainCamera.topDownViewMatrix();

        // Controls
        ImGuiIO io = ImGui::GetIO();
        m_window.updateInput();
        if (!io.WantCaptureMouse) { mainCamera.updateInput(); } // Prevent camera movement when accessing UI elements

        // Particle simulation
        particleEmitterManager.updateEmitters();

        // Render point lights shadow maps
        const glm::mat4 pointLightShadowMapsProjection = renderConfig.pointShadowMapsProjectionMatrix();
        glViewport(0, 0, utils::SHADOWTEX_WIDTH, utils::SHADOWTEX_HEIGHT); // Set viewport size to fit shadow map resolution
        for (size_t pointLightNum = 0U; pointLightNum < lightManager.numPointLights(); pointLightNum++) {
            const PointLight& light = lightManager.pointLightAt(pointLightNum);
            light.wipeFramebuffers();
            drawMeshTreePtL(scene.root,glm::mat4(1),light, m_pointShadowShader, pointLightShadowMapsProjection, renderConfig);
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
            drawMeshTreeAL(scene.root,glm::mat4(1),areaLightShadowMapsProjection, lightView, renderConfig);
        }

        // Render scene
        mainRenderer.render(viewProjectionMain, mainCamera.cameraPos());
        if (renderConfig.drawMinimap) { minimapRenderer.render(viewProjectionMinimap, mainCamera.topDownPos()); }

        // Draw UI
        glViewport(0, 0, utils::WIDTH, utils::HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        menu.draw(viewProjectionMain);

        // Process inputs and swap the window buffer
        m_window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
