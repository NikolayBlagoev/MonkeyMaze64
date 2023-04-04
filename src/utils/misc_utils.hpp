#ifndef _MISC_UTILS_HPP_
#define _MISC_UTILS_HPP_

#include <random>

namespace utils {
    /********** Random number generation **********/
    static std::random_device randomDevice;                 // Will be used to obtain a seed for the random number engine
    static std::mt19937 randomGenerator(randomDevice());    // Standard mersenne_twister_engine seeded with randomDevice

    template<typename T>
    static T randomNumber(T low, T high) { 
        std::uniform_real_distribution<T> distribution(low, high);
        return distribution(randomGenerator);
    }

    /********** Geometric utilities **********/
    static glm::vec4 quaternionToAxisAndDegrees(const glm::vec4& quaternion) {
        glm::vec4 in        = glm::normalize(quaternion);
        float angleRads     = acos(in.w) * 2.0f;
        float angleDegrees  = angleRads * 180.0f / glm::pi<float>();
        float x = in.x / sin(angleRads / 2.0f);
        float y = in.y / sin(angleRads / 2.0f);
        float z = in.z / sin(angleRads / 2.0f);
        return glm::vec4(x, y, z, angleDegrees);
    }
}

#endif
