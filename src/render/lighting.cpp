#include "lighting.h"

LightManager::LightManager() { glGenBuffers(1, &ssboPointLights); }

void LightManager::bind() {
    // Point lights
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPointLights);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * pointLights.size(), pointLights.data(), GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPointLights); // Bind to binding=0

    // Unbind
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0U);
}
