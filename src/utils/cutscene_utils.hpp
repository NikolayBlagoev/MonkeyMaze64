#ifndef _CUTSCENE_UTILS_HPP_
#define _CUTSCENE_UTILS_HPP_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
DISABLE_WARNINGS_POP()

#include <render/config.h>
#include <render/bezier.h>
#include <render/mesh_tree.h>
#include <ui/camera.h>

namespace utils {
    static void initCutscene(BezierCurveManager& bezierCurveManager, RenderConfig& renderConfig, LightManager& lightManager,
                             MeshTree* player, MeshTree* playerLight,
                             Camera& mainCamera, Camera& playerCamera, glm::vec3 playerPos) {
        // Disable user input and set camera slightly in front of player model, looking back at it
        renderConfig.controlPlayer = false;
        mainCamera.setUserInteraction(false);
        playerCamera.setUserInteraction(false);
        glm::vec3 xzPlaneForward    = playerCamera.getForward();
        xzPlaneForward.y            = 0.0f;
        xzPlaneForward              = glm::normalize(xzPlaneForward);
        mainCamera.setPosition(glm::vec3(0.0f, 1.0f, -utils::CUTSCENE_POSITION_OFFSET));
        mainCamera.setForward(glm::vec3(0.0f, 0.0f, 1.0f));
        playerCamera.setPosition(glm::vec3(0.0f));
        player->transform.translate = glm::vec3(0.0f, -0.1f, 0.0f);

        // Configure Hyper Sonic rip-off particle effects
        // Yes, I assume that playerLight has a particle generator because it's placed there in main. Sue me
        playerLight->particleEmitter->m_baseColor       = glm::vec4(0.5f, 0.5f, 0.5f, 0.2f);
        playerLight->particleEmitter->m_baseVelocity    = glm::vec3(0.0f, -0.03f, 0.0f);
        playerLight->particleEmitter->m_baseLife        = 42.0f;
        playerLight->particleEmitter->m_baseSize        = 0.03f;
        // playerLight->particleEmitter->m_position        = glm::vec3(0.0f, 3.0f, 0.0f);
        playerLight->transform.translate                = glm::vec3(0.0f, 10.0f, 0.0f);
        renderConfig.colorDeviation     = 0.5f;
        renderConfig.velocityDeviation  = 0.01f;
        renderConfig.lifeDeviation      = 50.0f;
        renderConfig.sizeDeviation      = 0.01f;

        // Turn off player lights
        // Yes, another valid pointer assumption. It's 3:59AM now. Deal. With. It.
        // EDIT: It's 4:00AM
        playerLight->pl->intensityMultiplier    = 0.0f;
    }
}

#endif
