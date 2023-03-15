#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <filesystem>

namespace utils {
    constexpr int32_t WIDTH         = 1920;
    constexpr int32_t HEIGHT        = 1080;
    constexpr float ASPECT_RATIO    = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
    
    const std::filesystem::path RESOURCES_DIR_PATH  = RESOURCES_DIR;
    const std::filesystem::path SHADERS_DIR_PATH    = SHADERS_DIR;
}

#endif
