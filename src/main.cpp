#include <render/config.h>
#include <render/deferred.h>
#include <render/lighting.h>
#include <render/mesh.h>
#include <render/scene.h>
#include <render/texture.h>
#include <ui/camera.h>
#include <ui/menu.h>
#include <utils/constants.h>

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

#include <functional>
#include <iostream>
#include <vector>

// Game state
// TODO: Have a separate struct for this if it becomes too much
bool cameraZoomed = false;

void onKeyPressed(int key, int mods) {
    switch (key) {
        case GLFW_KEY_LEFT_CONTROL:
            cameraZoomed = true;
            break;
    }
}

void onKeyReleased(int key, int mods) {
    switch (key) {
        case GLFW_KEY_LEFT_CONTROL:
            cameraZoomed = false;
            break;
    }
}

void onMouseMove(const glm::dvec2& cursorPos) {}

void onMouseClicked(int button, int mods) {}

void onMouseReleased(int button, int mods) {}

void keyCallback(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) { onKeyPressed(key, mods); }
    else if (action == GLFW_RELEASE) { onKeyReleased(key, mods); }
}

void mouseButtonCallback(int button, int action, int mods) {
    if (action == GLFW_PRESS) { onMouseClicked(button, mods); }
    else if (action == GLFW_RELEASE) { onMouseReleased(button, mods); }
}

int main(int argc, char* argv[]) {
    // World setup
    // * y is up *
    glm::vec3 characterStart = glm::vec3 (7.0f, 0.25f, 0.0f);
    glm::vec3 characterCameraStart = characterStart + glm::vec3(0.0f, 1.0f, 2.0f);

    // Init core objects
    Window m_window("Final Project", glm::ivec2(utils::WIDTH, utils::HEIGHT), OpenGLVersion::GL46);

    RenderConfig renderConfig;

    Camera freeCamera(&m_window, renderConfig, glm::vec3(3.0f, 3.0f, 3.0f), -glm::vec3(1.2f, 1.1f, 0.9f));
    Camera characterCamera(&m_window, renderConfig, characterCameraStart, characterStart - characterCameraStart);

    Scene scene;
    LightManager lightManager(renderConfig);
    DeferredRenderer deferredRenderer(renderConfig, scene, lightManager);
    Menu menu(scene, renderConfig, lightManager);

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

    // Add models and test texture
    scene.addMesh(utils::RESOURCES_DIR_PATH / "models" / "dragonWithFloor.obj");
    size_t characterId = scene.addMesh(utils::RESOURCES_DIR_PATH / "models" / "dragon.obj");
    Texture m_texture(utils::RESOURCES_DIR_PATH / "textures" / "checkerboard.png");

    scene.transformParams[characterId].translate += characterStart;

    // Add test lights
    lightManager.addPointLight(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    lightManager.addPointLight(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightManager.addAreaLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
    lightManager.addAreaLight(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f));

    // Main loop
    while (!m_window.shouldClose()) {
        // Set render config based and selected camera
        Camera& camera = renderConfig.controlCharacter ? characterCamera : freeCamera;

        // Controls, update game state
        ImGuiIO io = ImGui::GetIO();
        m_window.updateInput();
        if (!io.WantCaptureMouse) { // Prevent camera movement when accessing UI elements
            if (renderConfig.controlCharacter)
                camera.updateInput(scene, characterId);
            else
                camera.updateInput();
        }

        // Clear the screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // View-projection matrices setup
        const float fovRadians = glm::radians(cameraZoomed ? renderConfig.zoomedVerticalFOV : renderConfig.verticalFOV);
        const glm::mat4 m_viewProjectionMatrix = glm::perspective(fovRadians, utils::ASPECT_RATIO, 0.1f, 30.0f) * camera.viewMatrix();

        // Render point lights shadow maps
        const glm::mat4 pointLightShadowMapsProjection = renderConfig.pointShadowMapsProjectionMatrix();
        glViewport(0, 0, utils::SHADOWTEX_WIDTH, utils::SHADOWTEX_HEIGHT); // Set viewport size to fit shadow map resolution
        for (size_t pointLightNum = 0U; pointLightNum < lightManager.numPointLights(); pointLightNum++) {
            const PointLight& light = lightManager.pointLightAt(pointLightNum);
            light.wipeFramebuffers();

            // Render each model
            for (size_t modelNum = 0U; modelNum < scene.numMeshes(); modelNum++) {
                const GPUMesh& mesh                         = scene.meshAt(modelNum);
                const glm::mat4 modelMatrix                 = scene.modelMatrix(modelNum);
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
            for (size_t modelNum = 0U; modelNum < scene.numMeshes(); modelNum++) {
                const GPUMesh& mesh         = scene.meshAt(modelNum);
                const glm::mat4 modelMatrix = scene.modelMatrix(modelNum);

                // Bind light camera mvp matrix
                const glm::mat4 lightMvp = areaLightShadowMapsProjection * lightView *  modelMatrix;
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(lightMvp));

                // Bind model's VAO and draw its elements
                mesh.draw();
            }
        }

        // Render scene
        deferredRenderer.render(m_viewProjectionMatrix, camera.cameraPos());

        // Draw UI
        glViewport(0, 0, utils::WIDTH, utils::HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        menu.draw(m_viewProjectionMatrix);

        // Process inputs and swap the window buffer
        m_window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
