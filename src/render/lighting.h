#ifndef _LIGHTING_H_
#define _LIGHTING_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
#include <glm/vec4.hpp>
DISABLE_WARNINGS_POP()
#include <vector>

// Vec3 is a universal no-no due to alignment issues; you have been warned (https://stackoverflow.com/a/38172697/14247568)

// TODO: Expand to a shadowing point light w/ cubemap
struct PointLight {
    glm::vec4 position;
    glm::vec4 color;
};

class LightManager {
public:
    LightManager();

    void bind();

    void addPointLight(const PointLight& light) { pointLights.push_back(light); }
    void removePointLight(size_t idx)  { pointLights.erase(pointLights.begin() + idx); }
    size_t numPointLights() { return pointLights.size(); }
    PointLight& pointLightAt(size_t idx) { return pointLights[idx]; }

private:
    static constexpr GLuint INVALID = 0xFFFFFFFF;

    GLuint ssboPointLights { INVALID };
    std::vector<PointLight> pointLights;
};

#endif
