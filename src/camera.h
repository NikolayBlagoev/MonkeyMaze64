#pragma once

#include <vector>
#include <glm/vec4.hpp>

class CameraObj{
    public:
    CameraObj(glm::vec4* cam, glm::vec4* stand) : camera(cam), stand2(stand) {

    }
    glm::vec4* camera;
    glm::vec4* stand2;
    bool motion_left = true;
    bool motion_up = true;

};