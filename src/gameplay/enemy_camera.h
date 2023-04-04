#ifndef _ENEMY_CAMERA_H
#define _ENEMY_CAMERA_H

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec4.hpp>
DISABLE_WARNINGS_POP()

#include <vector>

class EnemyCamera {
public:
    EnemyCamera(glm::vec4* cam, glm::vec4* stand) : camera(cam), stand2(stand) {}

    glm::vec4* camera;
    glm::vec4* stand2;

    bool motion_left    = true;
    bool motion_up      = true;
};

#endif
