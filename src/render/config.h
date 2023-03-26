#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()

#include <utils/constants.h>

struct RenderConfig {
    bool controlCharacter   { true };

    // Camera (angles in degrees)
    float moveSpeed         { 0.05f };
    float lookSpeed         { 0.0015f };
    float verticalFOV       { 60.0f };
    float zoomedVerticalFOV { 35.0f };
    bool constrainVertical  { false };
    bool invertControls     { true };

    // Lighting debug
    bool drawLights { false };
    bool drawSelectedPointLight { false };
    bool drawSelectedAreaLight { false };

    // Shadow maps
    float areaShadowFovY    = 60.0f; // Degrees
    float shadowNearPlane   = 0.5f;
    float shadowFarPlane    = 30.0f;
    glm::mat4 areaShadowMapsProjectionMatrix() const {
        return glm::perspective(glm::radians(areaShadowFovY),
                                utils::SHADOW_ASPECT_RATIO,
                                shadowNearPlane, shadowFarPlane);
    }
    glm::mat4 pointShadowMapsProjectionMatrix() const {
        return glm::perspective(glm::radians(utils::CUBE_SHADOW_FOV),
                                utils::SHADOW_ASPECT_RATIO,
                                shadowNearPlane, shadowFarPlane);
    }
};

#endif
