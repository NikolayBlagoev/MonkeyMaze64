//#include "Image.h"
#include "render/mesh.h"
#include "render/scene.h"
#include "render/texture.h"
#include "ui/camera.h"
#include "ui/menu.h"
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

constexpr int32_t WIDTH         = 1920;
constexpr int32_t HEIGHT        = 1080;
constexpr float FOV             = glm::radians(90.0f);
constexpr float ASPECT_RATIO    = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
const std::filesystem::path RESOURCES_DIR_PATH  = RESOURCES_DIR;
const std::filesystem::path SHADERS_DIR_PATH    = SHADERS_DIR;

// In here you can handle key presses
// key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
// mods - Any modifier keys pressed, like shift or control
void onKeyPressed(int key, int mods)
{
    std::cout << "Key pressed: " << key << std::endl;
}

// In here you can handle key releases
// key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
// mods - Any modifier keys pressed, like shift or control
void onKeyReleased(int key, int mods)
{
    std::cout << "Key released: " << key << std::endl;
}

// If the mouse is moved this function will be called with the x, y screen-coordinates of the mouse
void onMouseMove(const glm::dvec2& cursorPos)
{
    std::cout << "Mouse at position: " << cursorPos.x << " " << cursorPos.y << std::endl;
}

// If one of the mouse buttons is pressed this function will be called
// button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
// mods - Any modifier buttons pressed
void onMouseClicked(int button, int mods)
{
    std::cout << "Pressed mouse button: " << button << std::endl;
}

// If one of the mouse buttons is released this function will be called
// button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
// mods - Any modifier buttons pressed
void onMouseReleased(int button, int mods)
{
    std::cout << "Released mouse button: " << button << std::endl;
}

void keyCallback(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) { onKeyPressed(key, mods); }
    else if (action == GLFW_RELEASE) { onKeyReleased(key, mods); }
}

void mouseButtonCallback(int button, int action, int mods) {
    if (action == GLFW_PRESS) { onMouseClicked(button, mods); }
    else if (action == GLFW_RELEASE) { onMouseReleased(button, mods); }
}

int main(int argc, char* argv[]) {
    // Init core objects
    Window m_window("Final Project", glm::ivec2(WIDTH, HEIGHT), OpenGLVersion::GL46);
    Camera mainCamera(&m_window, glm::vec3(3.0f, 3.0f, 3.0f), -glm::vec3(1.2f, 1.1f, 0.9f));
    Scene scene;
    Menu menu(scene);

    // Register UI callbacks
    m_window.registerKeyCallback(keyCallback);
    m_window.registerMouseMoveCallback(onMouseMove);
    m_window.registerMouseButtonCallback(mouseButtonCallback);

    // Build shaders
    Shader m_defaultShader;
    Shader m_shadowShader;
    try {
        ShaderBuilder defaultBuilder;
        defaultBuilder.addStage(GL_VERTEX_SHADER, SHADERS_DIR_PATH / "shader_vert.glsl");
        defaultBuilder.addStage(GL_FRAGMENT_SHADER, SHADERS_DIR_PATH / "shader_frag.glsl");
        m_defaultShader = defaultBuilder.build();

        ShaderBuilder shadowBuilder;
        shadowBuilder.addStage(GL_VERTEX_SHADER, SHADERS_DIR_PATH / "shadow_vert.glsl");
        m_shadowShader = shadowBuilder.build();
    } catch (ShaderLoadingException e) { std::cerr << e.what() << std::endl; }

    // Add models and test texture
    scene.addMesh(RESOURCES_DIR_PATH / "models" / "dragon.obj");
    scene.addMesh(RESOURCES_DIR_PATH / "models" / "dragon.obj");
    Texture m_texture(RESOURCES_DIR_PATH / "textures" / "checkerboard.png");

    // Main loop
    while (!m_window.shouldClose()) {
        // This is your game loop
        // Put your real-time logic and rendering in here
        m_window.updateInput();

        // Clear the screen
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ...
        glEnable(GL_DEPTH_TEST);

        // Controls and UI
        ImGuiIO io = ImGui::GetIO();
        menu.draw();
        if (!io.WantCaptureMouse) { mainCamera.updateInput(); }

        // Projection matrix setup
        const glm::mat4 m_projectionMatrix = glm::perspective(FOV, ASPECT_RATIO, 0.1f, 30.0f);

        for (size_t modelNum = 0U; modelNum < scene.numMeshes(); modelNum++) {
            const GPUMesh& mesh             = scene.meshAt(modelNum);
            const glm::mat4 modelMatrix    = scene.modelMatrix(modelNum);

            // Normals should be transformed differently than positions (ignoring translations + dealing with scaling):
            // https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
            const glm::mat4 mvpMatrix           = m_projectionMatrix * mainCamera.viewMatrix() * modelMatrix;
            const glm::mat3 normalModelMatrix   = glm::inverseTranspose(glm::mat3(modelMatrix));

            // Bind shader(s) and uniform(s)
            m_defaultShader.bind();
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
            if (mesh.hasTextureCoords()) {
                m_texture.bind(GL_TEXTURE0);
                glUniform1i(3, 0);
                glUniform1i(4, GL_TRUE);
            } else { glUniform1i(4, GL_FALSE); }

            mesh.draw();
        }

        // Processes input and swaps the window buffer
        m_window.swapBuffers();
    }

    return 0;
}
