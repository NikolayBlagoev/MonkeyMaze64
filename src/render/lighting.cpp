#include "lighting.h"

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

#include <utils/constants.h>

glm::vec3 AreaLight::forwardDirection() const {
    const glm::mat4 xRotation   = glm::rotate(glm::radians(rotX), s_xAxis);
    const glm::mat4 yRotation   = glm::rotate(glm::radians(rotY), s_yAxis);
    return glm::vec3(xRotation * yRotation * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
}

glm::mat4 AreaLight::viewMatrix() const {
    // Construct upward direction
    const glm::vec3 forward = forwardDirection();
    const glm::vec3 horAxis = glm::cross(s_yAxis, forward);
    const glm::vec3 up      = glm::normalize(glm::cross(forward, horAxis));

    return glm::lookAt(position, position + forward, up);
}

LightManager::LightManager(const RenderConfig& renderConfig) : m_renderConfig(renderConfig) { 
    glGenBuffers(1, &ssboPointLights);
    glGenBuffers(1, &ssboAreaLights);

    // 2D array texture for area light shadow maps
    glGenTextures(1, &shadowTexArr);
    glTextureParameteri(shadowTexArr, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Clamp coordinates outside of texture to [0, 1] range
    glTextureParameteri(shadowTexArr, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(shadowTexArr, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear interpolation of texels to allow for PCF
    glTextureParameteri(shadowTexArr, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(shadowTexArr, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE); // Set texture comparison to return fraction of neighbouring samples passing the below test (https://www.khronos.org/opengl/wiki/Sampler_Object#Comparison_mode)
    glTextureParameteri(shadowTexArr, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
}

LightManager::~LightManager() {
    glDeleteBuffers(1, &ssboPointLights);
    glDeleteBuffers(1, &ssboAreaLights);
    glDeleteTextures(1, &shadowTexArr);

    for (const AreaLight& areaLight : areaLights) { glDeleteFramebuffers(1, &areaLight.framebuffer); }
}

void LightManager::addAreaLight(const glm::vec3& position, const glm::vec3& color, float xAngle, float yAngle) {
    AreaLight light { position, xAngle, yAngle, INVALID, color };

    // Resize texture array to accomodate new shadow map
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F, utils::SHADOWTEX_WIDTH, utils::SHADOWTEX_HEIGHT, areaLights.size() + 1U);
    
    // Create framebuffer to draw to
    glCreateFramebuffers(1, &light.framebuffer);
    glNamedFramebufferTextureLayer(light.framebuffer, GL_DEPTH_ATTACHMENT, shadowTexArr, 0, areaLights.size());

    areaLights.push_back(light);
}

void LightManager::removeAreaLight(size_t idx) {
    // Resize texture array to conserve memory
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F, utils::SHADOWTEX_WIDTH, utils::SHADOWTEX_HEIGHT, areaLights.size() - 1U);

    // Destroy framebuffer corresponding to the shadow map
    const AreaLight& light = areaLights[idx];
    glDeleteFramebuffers(1, &light.framebuffer);

    areaLights.erase(areaLights.begin() + idx);
}

std::vector<AreaLightShader> LightManager::createAreaLightsShaderData(const glm::mat4& modelMatrix) {
    const glm::mat4 projection = m_renderConfig.shadowMapsProjectionMatrix();
    std::vector<AreaLightShader> shaderData(areaLights.size());
    for (const AreaLight& light : areaLights) {
        glm::mat4 mvp = projection * light.viewMatrix() * modelMatrix;
        shaderData.push_back({ glm::vec4(light.position, 0.0f), glm::vec4(light.color, 0.0f), mvp });
    }
    return shaderData;
}

void LightManager::bind(const glm::mat4& modelMatrix) {
    // Point lights
    glNamedBufferData(ssboPointLights, sizeof(PointLight) * pointLights.size(), pointLights.data(), GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPointLights); // Bind to binding=0

    // Area lights
    std::vector<AreaLightShader> areaLightsShaderData = createAreaLightsShaderData(modelMatrix);
    glNamedBufferData(ssboAreaLights, sizeof(AreaLightShader) * areaLightsShaderData.size(), areaLightsShaderData.data(), GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboAreaLights); // Bind to binding=1
}
