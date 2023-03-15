#ifndef _LIGHTING_H_
#define _LIGHTING_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
DISABLE_WARNINGS_POP()

#include <render/config.h>
#include <vector>

// Vec3 is a universal no-no due to alignment issues; you have been warned (https://stackoverflow.com/a/38172697/14247568)

// TODO: Expand to a shadowing point light w/ cubemap
struct PointLight {
    glm::vec4 position;
    glm::vec4 color;
};

// TODO: Expand with 1. Ability to use texture to define lighting 2. Spotlight functionality
struct AreaLight {
    static constexpr glm::vec3 s_xAxis { 1.0f, 0.0f, 0.0f };
    static constexpr glm::vec3 s_yAxis { 0.0f, 1.0f, 0.0f };

    // Spatial data (rotX and rotY are angles in degrees)
    glm::vec3 position;
    float rotX;
    float rotY;

    // OpenGL setup 
    GLuint framebuffer;

    // Lighting properties
    glm::vec3 color;

    glm::vec3 forwardDirection() const;
    glm::mat4 viewMatrix() const;
};

struct AreaLightShader {
    glm::vec4 position;
    glm::vec4 color;
    glm::mat4 mvp;
};

class LightManager {
public:
    LightManager(const RenderConfig& renderConfig);
    ~LightManager();

    void bind(const glm::mat4& modelMatrix);

    void addPointLight(const PointLight& light) { pointLights.push_back(light); }
    void removePointLight(size_t idx)  { pointLights.erase(pointLights.begin() + idx); }
    size_t numPointLights() { return pointLights.size(); }
    PointLight& pointLightAt(size_t idx) { return pointLights[idx]; }

    void addAreaLight(const glm::vec3& position, const glm::vec3& color, float xAngle = 0.0f, float yAngle = 0.0f);
    void removeAreaLight(size_t idx);
    size_t numAreaLights() { return areaLights.size(); }
    AreaLight& areaLightAt(size_t idx) { return areaLights[idx]; }
    std::vector<AreaLightShader> createAreaLightsShaderData(const glm::mat4& modelMatrix);

private:
    static constexpr GLuint INVALID = 0xFFFFFFFF;

    const RenderConfig& m_renderConfig;

    GLuint ssboPointLights { INVALID };
    std::vector<PointLight> pointLights;

    GLuint ssboAreaLights { INVALID };
    GLuint shadowTexArr { INVALID };
    std::vector<AreaLight> areaLights;
};

#endif
