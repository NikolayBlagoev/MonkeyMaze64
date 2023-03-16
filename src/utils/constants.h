#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <filesystem>

namespace utils {
    // Viewport parameters
    constexpr int32_t WIDTH         = 1920;
    constexpr int32_t HEIGHT        = 1040;
    constexpr float ASPECT_RATIO    = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);

    // Shadow maps parameters
    constexpr int32_t SHADOWTEX_WIDTH   = 1024;
    constexpr int32_t SHADOWTEX_HEIGHT  = 1024;
    constexpr float SHADOW_ASPECT_RATIO = static_cast<float>(SHADOWTEX_WIDTH) / static_cast<float>(SHADOWTEX_HEIGHT);
    constexpr int32_t SHADOW_START_IDX  = 64;
    constexpr int32_t MAX_SHADOW_MAPS   = 64;
    
    const std::filesystem::path RESOURCES_DIR_PATH  = RESOURCES_DIR;
    const std::filesystem::path SHADERS_DIR_PATH    = SHADERS_DIR;
}

#endif
