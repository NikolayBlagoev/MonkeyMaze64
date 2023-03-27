#include "lighting.h"

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()

#include <utils/constants.h>
#include <iostream>

void PointLight::wipeFramebuffers() const {
    for (size_t face = 0UL; face < 6UL; face++) { 
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[face]);
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::array<glm::mat4, 6UL> PointLight::viewMatrices() const {
    return { glm::lookAt(position, position + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)),     // Right
             glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)),     // Left
             glm::lookAt(position, position + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),     // Top
             glm::lookAt(position, position + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)),     // Bottom
             glm::lookAt(position, position + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)),     // Backward
             glm::lookAt(position, position + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)) };   // Forward
}

std::array<glm::mat4, 6UL> PointLight::genMvpMatrices(const glm::mat4& model, const glm::mat4& projection) const {
    std::array<glm::mat4, 6UL> views = viewMatrices();
    for (size_t idx = 0U; idx < views.size(); idx++) { views[idx] = projection * views[idx] * model; }
    return views;
}

glm::vec3 AreaLight::forwardDirection() const { 
    float radiansRotX = glm::radians(rotX);
    float radiansRotY = glm::radians(rotY);
    glm::vec3 forwardUnscaled = glm::angleAxis(radiansRotX, s_xAxis) * glm::angleAxis(radiansRotY, s_yAxis) * s_xAxis;
    return glm::normalize(forwardUnscaled);
}

glm::mat4 AreaLight::viewMatrix() const {
    // Construct upward direction
    const glm::vec3 forward = forwardDirection();
    const glm::vec3 horAxis = glm::cross(s_yAxis, forward);
    const glm::vec3 up      = glm::normalize(glm::cross(forward, horAxis));

    return glm::lookAt(position, position + forward, up);
}

LightManager::LightManager(const RenderConfig& renderConfig) : m_renderConfig(renderConfig) { 
    glCreateBuffers(1, &ssboPointLights);
    glCreateBuffers(1, &ssboAreaLights);

    // Cubemap texture array for point light shadow maps
    glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &pointShadowTexArr);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, pointShadowTexArr);
    glTextureParameteri(pointShadowTexArr, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(pointShadowTexArr, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(pointShadowTexArr, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(pointShadowTexArr, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear interpolation of texels to allow for PCF
    glTextureParameteri(pointShadowTexArr, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(pointShadowTexArr, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE); // Set texture comparison to return fraction of neighbouring samples passing the below test (https://www.khronos.org/opengl/wiki/Sampler_Object#Comparison_mode)
    glTextureParameteri(pointShadowTexArr, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    // 2D texture array for area light shadow maps
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &areaShadowTexArr);
    glBindTexture(GL_TEXTURE_2D_ARRAY, areaShadowTexArr);
    glTextureParameteri(areaShadowTexArr, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // Coordinates outside of [0, 1] range clamp to -MAX_FLOAT, so they always fail the depth test
    glTextureParameteri(areaShadowTexArr, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTextureParameterfv(areaShadowTexArr, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(-std::numeric_limits<float>::max())));
    glTextureParameteri(areaShadowTexArr, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear interpolation of texels to allow for PCF
    glTextureParameteri(areaShadowTexArr, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(areaShadowTexArr, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE); // Set texture comparison to return fraction of neighbouring samples passing the below test (https://www.khronos.org/opengl/wiki/Sampler_Object#Comparison_mode)
    glTextureParameteri(areaShadowTexArr, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}

LightManager::~LightManager() {
    glDeleteBuffers(1, &ssboPointLights);
    glDeleteBuffers(1, &ssboAreaLights);
    glDeleteTextures(1, &areaShadowTexArr);
    glDeleteTextures(1, &pointShadowTexArr);

    for (const AreaLight& areaLight : areaLights) { glDeleteFramebuffers(1, &areaLight.framebuffer); }
    for (const PointLight& pointLight : pointLights) { glDeleteFramebuffers(6, pointLight.framebuffers.data()); }
}

void LightManager::addPointLight(const glm::vec3& position, const glm::vec3& color, float intensityMultiplier) {
    PointLight light = { position, color, intensityMultiplier, {INVALID} };

    // Resize texture array to fit new shadowmap
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, pointShadowTexArr);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT32F, utils::SHADOWTEX_WIDTH, utils::SHADOWTEX_HEIGHT, static_cast<GLsizei>((pointLights.size() + 1UL) * 6UL), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Create framebuffer to draw to for each face
    glCreateFramebuffers(6, light.framebuffers.data());
    for (size_t face = 0UL; face < 6UL; face++) { glNamedFramebufferTextureLayer(light.framebuffers[face], GL_DEPTH_ATTACHMENT, pointShadowTexArr, 0, static_cast<GLint>((pointLights.size() * 6UL) + face)); }

    pointLights.push_back(light);
}

void LightManager::removePointLight(size_t idx) {
    // Resize texture array to save space
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, pointShadowTexArr);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT32F, utils::SHADOWTEX_WIDTH, utils::SHADOWTEX_HEIGHT, static_cast<GLsizei>((pointLights.size() - 1UL) * 6UL), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Destroy framebuffers corresponding to the shadow map
    const PointLight& light = pointLights[idx];
    glDeleteFramebuffers(6, light.framebuffers.data());

    pointLights.erase(pointLights.begin() + idx);

    // Reattach framebuffers to corresponding texture layers
    // Plonking out a framebuffer from the beginning or middle causes the framebuffers to become misaligned from their position in the array
    for (size_t lightIdx = 0UL; lightIdx < pointLights.size(); lightIdx++) {
        const PointLight& light = pointLights[lightIdx];
        for (size_t face = 0UL; face < 6UL; face++) { glNamedFramebufferTextureLayer(light.framebuffers[face], GL_DEPTH_ATTACHMENT, pointShadowTexArr, 0, static_cast<GLint>((lightIdx * 6UL) + face)); }
    }
}

std::vector<PointLightShader> LightManager::createPointLightsShaderData() {
    const glm::mat4 projection = m_renderConfig.pointShadowMapsProjectionMatrix();
    std::vector<PointLightShader> shaderData;
    for (const PointLight& light : pointLights) { shaderData.push_back({ glm::vec4(light.position, 0.0f), glm::vec4(light.color * light.intensityMultiplier, 0.0f) }); }
    return shaderData;
}

void LightManager::addAreaLight(const glm::vec3& position, const glm::vec3& color, float intensityMultiplier, float xAngle, float yAngle) {
    AreaLight light = { position, xAngle, yAngle, color, intensityMultiplier, INVALID };

    // Resize texture array to fit new shadowmap
    glBindTexture(GL_TEXTURE_2D_ARRAY, areaShadowTexArr);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, utils::SHADOWTEX_WIDTH, utils::SHADOWTEX_HEIGHT, static_cast<GLsizei>(areaLights.size() + 1UL), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Create framebuffer to draw to
    glCreateFramebuffers(1, &light.framebuffer);
    glNamedFramebufferTextureLayer(light.framebuffer, GL_DEPTH_ATTACHMENT, areaShadowTexArr, 0, static_cast<GLint>(areaLights.size()));

    areaLights.push_back(light);
}

void LightManager::removeAreaLight(size_t idx) {
    // Destroy framebuffer corresponding to the shadow map
    const AreaLight& light = areaLights[idx];
    glDeleteFramebuffers(1, &light.framebuffer);

    // Resize texture array to save space
    glBindTexture(GL_TEXTURE_2D_ARRAY, areaShadowTexArr);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, utils::SHADOWTEX_WIDTH, utils::SHADOWTEX_HEIGHT, static_cast<GLsizei>(areaLights.size() - 1UL), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    areaLights.erase(areaLights.begin() + idx);

    // Reattach framebuffers to corresponding texture layers
    // Plonking out a framebuffer from the beginning or middle causes the framebuffers to become misaligned from their position in the array
    for (size_t lightIdx = 0UL; lightIdx < areaLights.size(); lightIdx++) {
        const AreaLight& light = areaLights[lightIdx];
        glNamedFramebufferTextureLayer(light.framebuffer, GL_DEPTH_ATTACHMENT, areaShadowTexArr, 0, static_cast<GLint>(lightIdx));
    }
}

std::vector<AreaLightShader> LightManager::createAreaLightsShaderData() {
    const glm::mat4 projection = m_renderConfig.areaShadowMapsProjectionMatrix();
    std::vector<AreaLightShader> shaderData;
    for (const AreaLight& light : areaLights) {
        glm::mat4 viewProjection = projection * light.viewMatrix();
        shaderData.push_back({ glm::vec4(light.position, 0.0f), glm::vec4(light.color * light.intensityMultiplier, 0.0f), viewProjection });
    }
    return shaderData;
}

void LightManager::bind() {
    // Point lights
    std::vector<PointLightShader> pointLightsShaderData = createPointLightsShaderData();
    glNamedBufferData(ssboPointLights, sizeof(PointLightShader) * pointLightsShaderData.size(), pointLightsShaderData.data(), GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPointLights); // Bind to binding=0

    // Point lights shadow maps sampler
    glActiveTexture(GL_TEXTURE0 + utils::SHADOW_START_IDX);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, pointShadowTexArr);
    glUniform1i(7, utils::SHADOW_START_IDX);

    // Area lights
    std::vector<AreaLightShader> areaLightsShaderData = createAreaLightsShaderData();
    glNamedBufferData(ssboAreaLights, sizeof(AreaLightShader) * areaLightsShaderData.size(), areaLightsShaderData.data(), GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboAreaLights); // Bind to binding=1

    // Area lights shadow maps sampler
    glActiveTexture(GL_TEXTURE0 + utils::SHADOW_START_IDX + 1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, areaShadowTexArr);
    glUniform1i(8, utils::SHADOW_START_IDX + 1);
}
