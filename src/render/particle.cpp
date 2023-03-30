#include "particle.h"

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()

#include <utils/constants.h>
#include <utils/misc_utils.hpp>
#include <iostream>

ParticleEmitter::ParticleEmitter(const RenderConfig& renderConfig, glm::vec3 position)
    : m_position(position)
    , m_renderConfig(renderConfig) {
    for (size_t particleNum = 0UL; particleNum < utils::MAX_PARTICLES_PER_EMITTER; particleNum++) {
        // Generate particle data with minor random variation
        glm::vec3 velocity  = randomVelocity();
        glm::vec4 color     = randomColor();
        float life          = randomLife();
        float size          = randomSize();

        // Create new particle and corresponding shader particle
        particles.emplace_back(m_position, color, size, velocity, life);
        particlesShader.emplace_back(m_position, color, size);
    }
}

glm::vec3 ParticleEmitter::randomVelocity() const {
    return glm::vec3(std::max(0.0f, m_baseVelocity.x + utils::randomNumber(-m_renderConfig.velocityDeviation, m_renderConfig.velocityDeviation)),
                     std::max(0.0f, m_baseVelocity.y + utils::randomNumber(-m_renderConfig.velocityDeviation, m_renderConfig.velocityDeviation)),
                     std::max(0.0f, m_baseVelocity.z + utils::randomNumber(-m_renderConfig.velocityDeviation, m_renderConfig.velocityDeviation)));
}

glm::vec4 ParticleEmitter::randomColor() const {
    return glm::vec4(std::max(0.0f, m_baseColor.r + utils::randomNumber(-m_renderConfig.colorDeviation, m_renderConfig.colorDeviation)),
                     std::max(0.0f, m_baseColor.g + utils::randomNumber(-m_renderConfig.colorDeviation, m_renderConfig.colorDeviation)),
                     std::max(0.0f, m_baseColor.b + utils::randomNumber(-m_renderConfig.colorDeviation, m_renderConfig.colorDeviation)),
                     std::max(0.0f, m_baseColor.a + utils::randomNumber(-m_renderConfig.colorDeviation, m_renderConfig.colorDeviation)));
}

float ParticleEmitter::randomLife() const { return m_baseLife + utils::randomNumber(-m_renderConfig.lifeDeviation, m_renderConfig.lifeDeviation); }

float ParticleEmitter::randomSize() const { return m_baseSize + utils::randomNumber(-m_renderConfig.sizeDeviation, m_renderConfig.sizeDeviation); }


void ParticleEmitter::reviveParticle(size_t idx) {
    Particle& particle              = particles[idx];
    ParticleShader& particleShader  = particlesShader[idx];

    particle.position   = m_position;
    particle.color      = randomColor();
    particle.velocity   = randomVelocity();
    particle.life       = randomLife();
    particle.size       = randomSize();
    particleShader.position = particle.position;
    particleShader.color    = particle.color;
    particleShader.size     = particle.size;
}

void ParticleEmitter::update() {
    for (size_t particleIdx = 0UL; particleIdx < particles.size(); particleIdx++) {
        Particle& particle              = particles[particleIdx];
        ParticleShader& particleShader  = particlesShader[particleIdx];

        if ( particle.life > 0.0f ) {
            particle.position   += particle.velocity;
            particle.life       -= lifeDelta;
            particleShader.position = particle.position;
        } else { reviveParticle(particleIdx); }
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

void ParticleEmitterManager::draw(GLuint renderBuffer, const glm::mat4& viewProjectionMatrix) const {
    glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer);
    particleShader.bind();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    updateAndBindAttributeBuffers(viewProjectionMatrix);
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(emitters.size() * utils::MAX_PARTICLES_PER_EMITTER));
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleEmitterManager::genAttributeBuffers() {
    // VBO containing vertex data
    glCreateBuffers(1, &vertexVBO);
    glNamedBufferStorage(vertexVBO, sizeof(vertexBufferData), vertexBufferData.data(), GL_STATIC_DRAW);

    // VBO containing particle data
    glCreateBuffers(1, &particlesVBO);
    glNamedBufferData(particlesVBO, 0, nullptr, GL_STREAM_DRAW); // No elements at objection creation as there are no emitters
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
    std::vector<ParticleShader> sortedParticles = genSortedParticles(viewProjectionMatrix);

    // Copy particle data
    glNamedBufferData(particlesVBO, sortedParticles.size() * sizeof(ParticleShader), nullptr, GL_STREAM_DRAW);
    glNamedBufferSubData(particlesVBO, 0, sortedParticles.size() * sizeof(ParticleShader), sortedParticles.data());

    // Define inputs to vertex shader
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, particlesVBO);
    glVertexAttribPointer(1, sizeof(ParticleShader) / sizeof(GLfloat), GL_FLOAT, GL_FALSE, 0, nullptr);

    // Instancing attribute divisors
    glVertexAttribDivisor(0, 0);    // Always reuse the same 4 vertices
    glVertexAttribDivisor(1, 1);    // One piece of particle data per particle
}
