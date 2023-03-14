#ifndef _LIGHTING_H_
#define _LIGHTING_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <vector>

// TODO: Expand to a shadowing point light w/ cubemap
struct PointLight {
    glm::vec3 position;
    glm::vec3 colour;
};

class LightManager {
public:
    LightManager();

    void bind();

    void addPointLight(const PointLight& light) { pointLights.push_back(light); }
    void removePointLight(size_t idx)  { pointLights.erase(pointLights.begin() + idx); }
    size_t numPointsLights() { return pointLights.size(); }
    const PointLight& pointLightAt(size_t idx) { return pointLights[idx]; }

private:
    static constexpr GLuint INVALID = 0xFFFFFFFF;

    GLuint ssboPointLights { INVALID };
    std::vector<PointLight> pointLights;
};

#endif
