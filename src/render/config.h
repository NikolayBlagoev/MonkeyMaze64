#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()

#include <utils/constants.h>

struct RenderConfig {
    // Camera (angles in degrees)
    float moveSpeed         { 0.03f };
    float lookSpeed         { 0.0015f };
    float verticalFOV       { 60.0f };
    float zoomedVerticalFOV { 35.0f };
    bool constrainVertical  { false };

    // Lighting debug
    bool drawLights { false };
    bool drawSelectedPointLight { false };
    bool drawSelectedAreaLight { false };

    // Area light shadow maps
    float shadowFovY        = 60.0f; // Degrees
    float shadowNearPlane   = 0.1f;
    float shadowFarPlane    = 30.0f;
    glm::mat4 shadowMapsProjectionMatrix() const { return glm::perspective(glm::radians(shadowFovY), utils::SHADOW_ASPECT_RATIO,
                                                                           shadowNearPlane, shadowFarPlane); }
};

#endif
