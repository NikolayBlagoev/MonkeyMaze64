#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
DISABLE_WARNINGS_POP()

#include <stdint.h>
#include <utils/constants.h>

enum class LightingModel {
    LambertPhong,
    LambertBlinnPhong,
    Toon,
    XToon,
    PBR
};

struct RenderConfig {
    // Camera (angles in degrees)
    float moveSpeed         { 0.09f };
    float lookSpeed         { 0.0015f };
    float verticalFOV       { 60.0f };
    float zoomedVerticalFOV { 35.0f };
    bool constrainVertical  { false };
    bool invertControls     { true  };
    bool controlPlayer      { true };

    // Lighting
    LightingModel lightingModel     { LightingModel::PBR };
    uint32_t toonDiscretizeSteps    { 4U };
    float toonSpecularThreshold     { 0.49f };

    // Default materials
    glm::vec4 defaultAlbedo { 1.0f, 1.0f, 1.0f, 1.0f };
    float defaultMetallic   { 0.0f };
    float defaultRoughness  { 0.0f };
    float defaultAO         { 0.0f };

    // HDR tonemapping and gamma correction
    bool enableHdr  { true };
    float exposure  { 1.0f };
    float gamma     { 2.2f };

    // Bloom
    bool enableBloom            { true };
    float bloomBrightThreshold  { 1.0f };
    uint32_t bloomIterations    { 5U };

    // SSAO
    bool enableSSAO                 { true };
    int32_t ssaoSamples             { 32 };     // Can causes performance explosions at higher values
    int32_t ssaoKernelLength        { 4 };      // Must be even
    float ssaoRadius                { 0.25f };
    float ssaoBias                  { 0.05f }; 
    float ssaoPower                 { 6.5f };
    float ssaoOcclussionCoefficient { 0.5f };

    // Particles
    float velocityDeviation { 0.005f };
    float colorDeviation    { 0.1f };
    float lifeDeviation     { 25.0f };
    float sizeDeviation     { 0.005f };

    // Parallax mapping
    float heightScale       { 0.025f };
    float minDepthLayers    { 8.0f };
    float maxDepthLayers    { 32.0f };

    // Lighting debug
    bool drawLights             { false };
    bool drawSelectedPointLight { false };
    bool drawSelectedAreaLight  { false };

    // Particles debug
    bool drawParticleEmitters           { false };
    bool drawSelectedParticleEmitter    { false };

    // Shadow maps
    float areaShadowFovY    { 60.0f }; // Degrees
    float shadowNearPlane   { 0.5f };
    float shadowFarPlane    { 30.0f };

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
