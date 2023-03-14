#ifndef _MENU_H_
#define _MENU_H_

#include <render/config.h>
#include <render/lighting.h>
#include <render/scene.h>

class Menu {
public:
    Menu(Scene& scene, RenderConfig& renderConfig, LightManager& lightManager);

    void draw();

private:
    void drawCameraTab();
    void addMesh();
    void drawMeshTab();
    void drawPointLightControls();
    void drawLightTab();

    Scene& m_scene;
    RenderConfig& m_renderConfig;
    LightManager& m_lightManager;

    size_t selectedMesh { 0U };
    size_t selectedPointLight { 0U };
};

#endif
