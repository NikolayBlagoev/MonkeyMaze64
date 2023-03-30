#ifndef _MISC_UTILS_HPP_
#define _MISC_UTILS_HPP_

#include <random>

namespace utils {
    static std::random_device randomDevice;                 // Will be used to obtain a seed for the random number engine
    static std::mt19937 randomGenerator(randomDevice());    // Standard mersenne_twister_engine seeded with randomDevice

    template<typename T>
    T randomNumber(T low, T high) { 
        std::uniform_real_distribution<T> distribution(low, high);
        return distribution(randomGenerator);
    }
}


#endif
