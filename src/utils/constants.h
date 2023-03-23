#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <filesystem>

namespace utils {
    // Viewport parameters
    constexpr int32_t WIDTH         = 1920;
    constexpr int32_t HEIGHT        = 1040;
    constexpr float ASPECT_RATIO    = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);

    // Deferred rendering parameters
    constexpr int32_t G_BUFFER_START_IDX = 48;

    // Shadow maps parameters
    constexpr int32_t SHADOWTEX_WIDTH   = 1024;
    constexpr int32_t SHADOWTEX_HEIGHT  = 1024;
    constexpr float SHADOW_ASPECT_RATIO = static_cast<float>(SHADOWTEX_WIDTH) / static_cast<float>(SHADOWTEX_HEIGHT);
    constexpr int32_t SHADOW_START_IDX  = 64;
    constexpr float CUBE_SHADOW_FOV     = 90.0f; // Must be 90 degrees so that the cameras on the six sides of the cube touch each other
    
    const std::filesystem::path RESOURCES_DIR_PATH  = RESOURCES_DIR;
    const std::filesystem::path SHADERS_DIR_PATH    = SHADERS_DIR;
}

#endif
