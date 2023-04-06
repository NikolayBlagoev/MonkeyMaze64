#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <filesystem>

namespace utils {
    // Viewport parameters
    constexpr int32_t WIDTH         = 1920;
    constexpr int32_t HEIGHT        = 1040;
    constexpr float ASPECT_RATIO    = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);

    // Minimap parameters
    constexpr float MINIMAP_SIZE_SCALE = 3.0f;

    // Rendering parameters
    constexpr int32_t LIGHTING_TEX_START_IDX        = 24;
    constexpr int32_t POST_PROCESSING_TEX_START_IDX = 32;
    constexpr int32_t G_BUFFER_TEX_START_IDX        = 48;
    constexpr int32_t HDR_BUFFER_TEX_START_IDX      = 56;

    // Shadow maps parameters
    constexpr int32_t SHADOWTEX_WIDTH               = 1024;
    constexpr int32_t SHADOWTEX_HEIGHT              = 1024;
    constexpr float SHADOW_ASPECT_RATIO             = static_cast<float>(SHADOWTEX_WIDTH) / static_cast<float>(SHADOWTEX_HEIGHT);
    constexpr int32_t SHADOW_START_IDX              = 64;
    constexpr float CUBE_SHADOW_FOV                 = 90.0f; // Must be 90 degrees so that the cameras on the six sides of the cube touch each other
    constexpr glm::vec3 CONSTANT_AREA_LIGHT_FALLOFF = { -1.0f, 0.0f, 0.0f };

    // Particles parameters
    constexpr size_t MAX_PARTICLES_PER_EMITTER = 512UL;

    // Grid generation parameters
    constexpr size_t TILES_PER_ROW  = 7UL; // Must be at least 2
    constexpr float TILE_LENGTH_X   = 7.72f;
    constexpr float TILE_LENGTH_Z   = 7.72f;

    // Gameplay parameters
    constexpr uint32_t NUM_HEADS_TO_COLLECT         = 7UL;
    constexpr float CUTSCENE_POSITION_OFFSET        = 3.0f;
    constexpr size_t SEIZURE_SUB_LIMIT              = 15UL;
    
    const std::filesystem::path RESOURCES_DIR_PATH  = RESOURCES_DIR;
    const std::filesystem::path SHADERS_DIR_PATH    = SHADERS_DIR;
}

#endif
