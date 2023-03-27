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
#include <array>
#include <vector>

// Vec3 is a universal no-no due to alignment issues; you have been warned (https://stackoverflow.com/a/38172697/14247568)

struct PointLight {
    // Spatial data
    glm::vec3 position;

    // Lighting properties
    glm::vec3 color;
    float intensityMultiplier { 1.0f }; // Color is multiplied by this value before being sent to shader (intended for usage w/ HDR rendering)
    
    // OpenGL setup 
    std::array<GLuint, 6UL> framebuffers;

    void wipeFramebuffers() const;
    std::array<glm::mat4, 6UL> viewMatrices() const;
    std::array<glm::mat4, 6UL> genMvpMatrices(const glm::mat4& model, const glm::mat4& projection) const;
};

struct PointLightShader {
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

    // Lighting properties
    glm::vec3 color;
    float intensityMultiplier { 1.0f }; // Color is multiplied by this value before being sent to shader (intended for usage w/ HDR rendering)

    // OpenGL setup 
    GLuint framebuffer;

    glm::vec3 forwardDirection() const;
    glm::mat4 viewMatrix() const;
};

struct AreaLightShader {
    glm::vec4 position;
    glm::vec4 color;
    glm::mat4 viewProjection;
};

class LightManager {
public:
    LightManager(const RenderConfig& renderConfig);
    ~LightManager();

    void bind();

    void addPointLight(const glm::vec3& position, const glm::vec3& color, float intensityMultiplier = 1.0f);
    void removePointLight(size_t idx);
    size_t numPointLights() { return pointLights.size(); }
    PointLight& pointLightAt(size_t idx) { return pointLights[idx]; }
    std::vector<PointLightShader> createPointLightsShaderData();

    void addAreaLight(const glm::vec3& position, const glm::vec3& color, float intensityMultiplier = 1.0f, float xAngle = 0.0f, float yAngle = 0.0f);
    void removeAreaLight(size_t idx);
    size_t numAreaLights() { return areaLights.size(); }
    AreaLight& areaLightAt(size_t idx) { return areaLights[idx]; }
    std::vector<AreaLightShader> createAreaLightsShaderData();

private:
    static constexpr GLuint INVALID = 0xFFFFFFFF;

    const RenderConfig& m_renderConfig;

    GLuint ssboPointLights { INVALID };
    GLuint pointShadowTexArr { INVALID };
    std::vector<PointLight> pointLights;

    GLuint ssboAreaLights { INVALID };
    GLuint areaShadowTexArr { INVALID };
    std::vector<AreaLight> areaLights;
};

#endif
