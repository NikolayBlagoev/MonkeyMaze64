#ifndef _MENU_H_
#define _MENU_H_

#include <render/config.h>
#include <render/scene.h>

class Menu {
public:
    Menu(Scene& scene, RenderConfig& renderConfig);

    void draw();

private:
    void drawCameraTab();
    void addMesh();
    void drawMeshTab();

    Scene& m_scene;
    RenderConfig& m_renderConfig;

    size_t selectedMesh { 0U };
};

#endif
