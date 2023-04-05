#ifndef _ENEMY_CAMERA_H
#define _ENEMY_CAMERA_H

#include <render/config.h>

// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/window.h>
#include <vector>

class EnemyCamera {
public:
    EnemyCamera(glm::vec4* cam, glm::vec4* stand, glm::vec3* cam_forward, glm::vec3* position, glm::vec3* color) : camera(cam), stand2(stand), cam_forward(cam_forward), position(position), color(color) {}

    glm::vec4* camera;
    glm::vec4* stand2;
    glm::vec3* cam_forward {nullptr};
    glm::vec3* position  {nullptr};
    glm::vec3* color {nullptr};
    bool motion_left    = true;
    bool motion_up      = true;

    bool canSeePoint(glm::vec3 point) {
        float angle = glm::acos(glm::dot(glm::normalize(point - *position), glm::normalize(*cam_forward)));

        return glm::degrees(angle) <= 55;
    };

    bool canSeePoint(glm::vec3 point, float maxDist) {
        float dist = glm::length(point - *position);

        return dist <= maxDist && canSeePoint(point);
    };
};

#endif
