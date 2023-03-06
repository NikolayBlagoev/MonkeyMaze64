#ifndef _MENU_H_
#define _MENU_H_

#include <render/scene.h>

class Menu {
public:
    Menu(Scene& scene);

    void draw();

private:
    void addObject();
    void drawObjectControls();

    Scene& m_scene;
    size_t selectedMesh { 0U };
};

#endif
