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
    void drawLightTab();

    // 3D debug view
    void drawLights(const glm::mat4& cameraMVP);

    Scene& m_scene;
    RenderConfig& m_renderConfig;
    LightManager& m_lightManager;

    Shader debugShader;

    size_t selectedMesh { 0U };
    size_t selectedPointLight { 0U };
};

#endif
