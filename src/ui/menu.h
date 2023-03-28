#ifndef _MENU_H_
#define _MENU_H_

#include <framework/shader.h>
#include <render/config.h>
#include <render/lighting.h>
#include <render/scene.h>

class Menu {
public:
    Menu(Scene& scene, RenderConfig& renderConfig, LightManager& lightManager);

    void draw(const glm::mat4& cameraMVP);

private:
    // Main drawing functions
    void draw2D();
    void draw3D(const glm::mat4& cameraMVP);

    // 2D UI
    void drawCameraTab();
    void addMesh();
    void drawMeshTab();
    void drawGeneralLightControls();
    void drawPointLightControls();
    void drawAreaLightControls();
    void drawLightTab();
    void drawShadowTab();

    // 3D debug view
    void drawPoint(float radius, const glm::vec4& screenPos, const glm::vec4& color);
    void drawPoint(float radius, const glm::vec4& screenPos, const glm::vec3& color) { drawPoint(radius, screenPos, glm::vec4(color, 1.0f)); }
    void drawLights(const glm::mat4& cameraMVP);

    Scene& m_scene;
    RenderConfig& m_renderConfig;
    LightManager& m_lightManager;

    Shader pointDebugShader;
    Shader lineDebugShader;

    int selectedMesh            { 0 };
    size_t selectedPointLight   { 0U };
    size_t selectedAreaLight    { 0U };
};

#endif
