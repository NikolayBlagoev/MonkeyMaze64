#include "particle.h"

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()

#include <utils/constants.h>
#include <utils/misc_utils.hpp>
#include <iostream>

ParticleEmitter::ParticleEmitter(glm::vec3 position, float velocityDeviation, float colorDeviation, float lifeDeviation, float sizeDeviation)
    : m_position(position) {
    for (size_t particleNum = 0UL; particleNum < utils::MAX_PARTICLES_PER_EMITTER; particleNum++) {
        // Generate particle data with minor random variation
        glm::vec3 velocity  = randomVelocity(velocityDeviation);
        glm::vec4 color     = randomColor(colorDeviation);
        float life          = randomLife(lifeDeviation);
        float size          = randomSize(sizeDeviation);

        // Create new particle and corresponding shader particle
        particles.emplace_back(m_position, color, size, velocity, life);
        particlesShader.emplace_back(m_position, color, size);
    }
}

glm::vec3 ParticleEmitter::randomVelocity(float velocityDeviation) const {
    return glm::vec3(m_baseVelocity.x + utils::randomNumber(-velocityDeviation, velocityDeviation),
                     m_baseVelocity.y + utils::randomNumber(-velocityDeviation, velocityDeviation),
                     m_baseVelocity.z + utils::randomNumber(-velocityDeviation, velocityDeviation));
}

glm::vec4 ParticleEmitter::randomColor(float colorDeviation) const {
    return glm::vec4(std::clamp(m_baseColor.r + utils::randomNumber(-colorDeviation, colorDeviation), 0.0f, 1.0f),
                     std::clamp(m_baseColor.g + utils::randomNumber(-colorDeviation, colorDeviation), 0.0f, 1.0f),
                     std::clamp(m_baseColor.b + utils::randomNumber(-colorDeviation, colorDeviation), 0.0f, 1.0f),
                     std::clamp(m_baseColor.a + utils::randomNumber(-colorDeviation, colorDeviation), 0.0f, 1.0f));
}

float ParticleEmitter::randomLife(float lifeDeviation) const { return m_baseLife + utils::randomNumber(-lifeDeviation, lifeDeviation); }

float ParticleEmitter::randomSize(float sizeDeviation) const { return m_baseSize + utils::randomNumber(-sizeDeviation, sizeDeviation); }


void ParticleEmitter::reviveParticle(size_t idx, float velocityDeviation, float colorDeviation, float lifeDeviation, float sizeDeviation) {
    Particle& particle              = particles[idx];
    ParticleShader& particleShader  = particlesShader[idx];

    particle.position   = m_position;
    particle.color      = randomColor(colorDeviation);
    particle.velocity   = randomVelocity(velocityDeviation);
    particle.life       = randomLife(lifeDeviation);
    particle.size       = randomSize(sizeDeviation);
    particleShader.position = particle.position;
    particleShader.color    = particle.color;
    particleShader.size     = particle.size;
}

void ParticleEmitter::update(float velocityDeviation, float colorDeviation, float lifeDeviation, float sizeDeviation) {
    for (size_t particleIdx = 0UL; particleIdx < particles.size(); particleIdx++) {
        Particle& particle              = particles[particleIdx];
        ParticleShader& particleShader  = particlesShader[particleIdx];

        if ( particle.life > 0.0f ) {
            particle.position   += particle.velocity;
            particle.life       -= lifeDelta;
            particleShader.position = particle.position;
        } else { reviveParticle(particleIdx, velocityDeviation, colorDeviation, lifeDeviation, sizeDeviation); }
    }
}

ParticleEmitterManager::ParticleEmitterManager(const RenderConfig& renderConfig) : m_renderConfig(renderConfig) { 
    genAttributeBuffers();
    try {
        ShaderBuilder particleShaderBuilder;
        particleShaderBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "particles" / "particles.vert");
        particleShaderBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "particles" / "particles.frag");
        particleShader = particleShaderBuilder.build();
    } catch (ShaderLoadingException e) { std::cerr << e.what() << std::endl; }
}

void ParticleEmitterManager::render(GLuint renderBuffer, const glm::mat4& viewProjectionMatrix) const {
    glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer);
    particleShader.bind();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    updateAndBindAttributeBuffers(viewProjectionMatrix);
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(emitters.size() * utils::MAX_PARTICLES_PER_EMITTER));
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
}

void ParticleEmitterManager::genAttributeBuffers() {
    // VAO holding all particle data
    glCreateVertexArrays(1, &VAO);

    // Create and specify format of VBO containing vertex data
    glCreateBuffers(1, &vertexVBO);
    glNamedBufferStorage(vertexVBO, sizeof(vertexBufferData), vertexBufferData.data(), 0);
    glVertexArrayVertexBuffer(VAO, 0, vertexVBO, 0, 3 * sizeof(GLfloat));
    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

    // Create and specify format of VBO containing particle-specific data
    glCreateBuffers(1, &particleVBO);
    glVertexArrayVertexBuffer(VAO, 1, particleVBO, 0, sizeof(ParticleShader));
    glEnableVertexArrayAttrib(VAO, 1);
    glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(ParticleShader, position));
    glEnableVertexArrayAttrib(VAO, 2);
    glVertexArrayAttribFormat(VAO, 2, 4, GL_FLOAT, GL_FALSE, offsetof(ParticleShader, color));
    glEnableVertexArrayAttrib(VAO, 3);
    glVertexArrayAttribFormat(VAO, 3, 1, GL_FLOAT, GL_FALSE, offsetof(ParticleShader, size));

    // Tell OpenGL which VBO to get data at each location from
    glVertexArrayAttribBinding(VAO, 0, 0);
    glVertexArrayAttribBinding(VAO, 1, 1);
    glVertexArrayAttribBinding(VAO, 2, 1);
    glVertexArrayAttribBinding(VAO, 3, 1);
    glVertexArrayAttribBinding(VAO, 4, 1);

    // Instancing attribute divisors (tell OpenGL how frequently to update vertex attributes)
    glVertexArrayBindingDivisor(VAO, 0, 0); // Reuse vertices for each index
    glVertexArrayBindingDivisor(VAO, 1, 1); // Update particle data by one for each instance (i.e. each particles gets ITS OWN data ONLY)
    glVertexArrayBindingDivisor(VAO, 2, 1); 
    glVertexArrayBindingDivisor(VAO, 3, 1); 
    glVertexArrayBindingDivisor(VAO, 4, 1);
}

std::vector<ParticleShader> ParticleEmitterManager::genSortedParticles(const glm::mat4& viewProjectionMatrix) const {
    // Copy over particles from all emitters
    std::vector<ParticleShader> sortedParticles;
    for (const ParticleEmitter& emitter : emitters) {
        std::copy(emitter.particlesShader.begin(), emitter.particlesShader.end(), std::back_inserter(sortedParticles));
    }

    // Sort by reverse camera-view depth (furthest first) and return
    std::sort(sortedParticles.begin(), sortedParticles.end(),
              [viewProjectionMatrix](ParticleShader left, ParticleShader right) { 
                            left.position   = glm::vec3(viewProjectionMatrix * glm::vec4(left.position, 1.0f));
                            right.position  = glm::vec3(viewProjectionMatrix * glm::vec4(right.position, 1.0f));
                            return left.position.z > right.position.z;
                        });
    return sortedParticles;
}

void ParticleEmitterManager::updateAndBindAttributeBuffers(const glm::mat4& viewProjectionMatrix) const {
    // Copy sorted particle data and bind VAO
    std::vector<ParticleShader> sortedParticles = genSortedParticles(viewProjectionMatrix);
    glNamedBufferData(particleVBO, sortedParticles.size() * sizeof(ParticleShader), sortedParticles.data(), GL_STREAM_DRAW);
    glBindVertexArray(VAO);
}
