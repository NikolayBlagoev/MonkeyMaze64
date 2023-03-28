#include <render/config.h>
#include <render/deferred.h>
#include <render/lighting.h>
#include <render/mesh.h>
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
#include <movements/bezier.h>
#include <sys/time.h>
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
glm::vec3 offsetBoard (-6.2f,0.f,-6.2f);
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
    signalChange();
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
    const MeshTransform& meshTransform = {mt->scale, mt->selfRotate, mt->rotateParent, mt->offset};
   
    glm::mat4 finalTransform = glm::rotate(currTransform, glm::radians(meshTransform.rotateParent.x), glm::vec3(1.0f, 0.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotateParent.y), glm::vec3(0.0f, 1.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotateParent.z), glm::vec3(0.0f, 0.0f, 1.0f));


    finalTransform = glm::translate(finalTransform, meshTransform.translate);

    // Rotate
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.z), glm::vec3(0.0f, 0.0f, 1.0f));
    const glm::mat4& modelMatrix = glm::scale(finalTransform, meshTransform.scale);
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
    const MeshTransform& meshTransform = {mt->scale, mt->selfRotate, mt->rotateParent, mt->offset};
   
    glm::mat4 finalTransform = glm::rotate(currTransform, glm::radians(meshTransform.rotateParent.x), glm::vec3(1.0f, 0.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotateParent.y), glm::vec3(0.0f, 1.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotateParent.z), glm::vec3(0.0f, 0.0f, 1.0f));


    finalTransform = glm::translate(finalTransform, meshTransform.translate);

    // Rotate
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.z), glm::vec3(0.0f, 0.0f, 1.0f));
    const glm::mat4& modelMatrix = glm::scale(finalTransform, meshTransform.scale);
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
                     glm::vec3 tr, glm::vec3 selfRot, glm::vec3 parRot, glm::vec3 scl){
    MeshTree* ret = new MeshTree(stand1, tr, selfRot, parRot,  scl);
    MeshTree* stand2m = new MeshTree(stand2, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f));
    ret->addChild(stand2m);
    MeshTree* cameram = new MeshTree(camera, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(0.f, 0.f, -20.f), glm::vec3(1.f));
    stand2m->addChild(cameram);
    MeshTree* aperturem = new MeshTree(aperture, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f));
    cameram->addChild(aperturem);
    CameraObj* cam = new CameraObj(cameram, stand2m, ret);
    return cam;
}

int main() {
    // Init core objects
    *ready = false;
    std::thread worker(worker_thread);
    
    RenderConfig renderConfig;
    Window m_window("Final Project", glm::ivec2(utils::WIDTH, utils::HEIGHT), OpenGLVersion::GL46);
    Camera mainCamera(&m_window, renderConfig, glm::vec3(3.0f, 3.0f, 3.0f), -glm::vec3(1.2f, 1.1f, 0.9f));
    TextureManager textureManager;
    Scene scene;
    LightManager lightManager(renderConfig);
    std::weak_ptr<const Texture> xToonTex = textureManager.addTexture(utils::RESOURCES_DIR_PATH / "textures" / "toon_map.png");
    DeferredRenderer deferredRenderer(renderConfig, scene, lightManager, xToonTex);
    Menu menu(scene, renderConfig, lightManager, deferredRenderer);

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
    MeshTree* drg = new MeshTree(&dragon, glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f));
    scene.addMesh(drg);

    GPUMesh camera = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "camera.obj");
    

    GPUMesh stand2 = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "stand2.obj");
    GPUMesh stand1 = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "stand1.obj");
    GPUMesh aperture = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "aperture.obj");
    

    GPUMesh crossing = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "crossing.obj");
    GPUMesh room = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "room.obj");
    GPUMesh tjunction = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "tjunction.obj");
    GPUMesh tunnel = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "tunnel.obj");
    GPUMesh turn = GPUMesh(utils::RESOURCES_DIR_PATH / "models" / "turn.obj");
    // MeshTree* dummyroom = new MeshTree(&room, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(0.3f));
    // scene.root->addChild(dummyroom);
    // // glm::vec3(-9.9f, 9.f, 0.f)
    // CameraObj* dummycam = makeCamera(&aperture, &camera, &stand2, &stand1, glm::vec3(-9.9f, 9.f, 0.f), glm::vec3(0.f), glm::vec3(0.f, 90.f, 0.f), glm::vec3(1.f));
    // dummyroom->addChild(dummycam->root);
    
    // Texture m_texture(utils::RESOURCES_DIR_PATH / "textures" / "checkerboard.png");

    // Add test lights
    lightManager.addPointLight(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    lightManager.addPointLight(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightManager.addAreaLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
    lightManager.addAreaLight(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f));
    MeshTree* boardRoot = new MeshTree();
    boardRoot->offset = offsetBoard;
    // Main loop
    std::chrono::high_resolution_clock timer; 
    using ms = std::chrono::microseconds;
    
    
    BezierCurve3d b3d = BezierCurve3d(glm::vec3(0.f), glm::vec3(1.f , 1.f, 0.f), glm::vec3(-1.f , 2.f, 0.f), glm::vec3(0.f, 3.f, 0.f), 10.f);
    BezierCurve3d b3d2 = BezierCurve3d(glm::vec3(0.f, 3.f, 0.f), glm::vec3(-1.f , 2.f, 0.f), glm::vec3(1.f , 1.f, 0.f), glm::vec3(0.f), 10.f);
 
    CompositeBezier3d b3c = CompositeBezier3d({&b3d,&b3d2}, true, 20.f);
    std::chrono::time_point millisec_since_epoch = timer.now();
    b3c.start_time = millisec_since_epoch;
    // b3d.total_time = 10.f; // in seconds
    bool flag = true;
    while (!m_window.shouldClose()) {
        millisec_since_epoch = timer.now();
        float delta = std::chrono::duration<float>(millisec_since_epoch - b3c.start_time).count();
        drg->offset=b3c.pos_t(delta);
        // std::cout<< drg->offset.y<< " "<< delta <<" "<< CLOCKS_PER_SEC << std::endl;
        // // b3d.prev_time = millisec_since_epoch;
        // Clear the screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        if (false){
            flag = false;
            free(boardRoot);
            float factorx = 7.72f;
            float factory = 7.72f;
            boardRoot = new MeshTree();
            for(int i = 0; i < 7; i ++){
                // glm::vec3 offset(0.f,10.f*i,0.f);
                for(int j = 0; j < 7; j++){
                    if(boardCopy[i][j]->tileType == 1){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 2){
                        boardRoot->addChild(new MeshTree(&room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 3){
                        boardRoot->addChild(new MeshTree(&room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f, 90.f, 0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 4){
                        boardRoot->addChild(new MeshTree(&room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f, 180.f, 0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 5){
                        boardRoot->addChild(new MeshTree(&room, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f, 270.f, 0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 6){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 7){
                        boardRoot->addChild(new MeshTree(&tunnel, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 8){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 9){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 10){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 11){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 12){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 13){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 14){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 15){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 16){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 17){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    }else if(boardCopy[i][j]->tileType == 18){
                        boardRoot->addChild(new MeshTree(&crossing, glm::vec3(factorx*i, 0.f, factory*j), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.3f)));
                    } 
                }
            }
            scene.root->addChild(boardRoot);

        }
        // View-projection matrices setup
        const float fovRadians = glm::radians(cameraZoomed ? renderConfig.zoomedVerticalFOV : renderConfig.verticalFOV);
        const glm::mat4 m_viewProjectionMatrix = glm::perspective(fovRadians, utils::ASPECT_RATIO, 0.1f, 30.0f) * mainCamera.viewMatrix();

        // Controls
        ImGuiIO io = ImGui::GetIO();
        m_window.updateInput();
        if (!io.WantCaptureMouse) { mainCamera.updateInput(); } // Prevent camera movement when accessing UI elements

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
        deferredRenderer.render(m_viewProjectionMatrix, mainCamera.cameraPos());

        // Draw UI
        glViewport(0, 0, utils::WIDTH, utils::HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        menu.draw(m_viewProjectionMatrix);

        // Process inputs and swap the window buffer
        m_window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
