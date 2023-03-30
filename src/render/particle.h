#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>

#include <render/config.h>
#include <array>
#include <stdint.h>
#include <vector>

// TODO: Expand with ability to use texture to define color
struct Particle {
    // Shader data
    glm::vec3 position;
    glm::vec4 color;
    float size;

    glm::vec3 velocity;
    float life;
};

struct ParticleShader {
    glm::vec3 position;
    glm::vec4 color;
    float size;

    constexpr auto operator<=>(const ParticleShader& other) const { position.z <=> other.position.z; }
};

class ParticleEmitter {
public:
    ParticleEmitter(glm::vec3 position, float velocityDeviation, float colorDeviation, float lifeDeviation, float sizeDeviation);

    void update(float velocityDeviation, float colorDeviation, float lifeDeviation, float sizeDeviation);

    // Initial values used for initialisation and resurrection of particles
    glm::vec3 m_baseVelocity    { 0.5f, 0.5f, 0.5f };
    glm::vec4 m_baseColor       { 1.0f, 1.0f, 1.0f, 0.01f };
    float m_baseLife            { 200.0f };
    float m_baseSize            { 2.0f };

    float lifeDelta     { 0.01f };
    glm::vec3 m_position;

    std::vector<Particle> particles;
    std::vector<ParticleShader> particlesShader;

private:
    glm::vec3 randomVelocity(float velocityDeviation) const;
    glm::vec4 randomColor(float colorDeviation) const;
    float randomLife(float lifeDeviation) const;
    float randomSize(float sizeDeviation) const;
    void reviveParticle(size_t idx, float velocityDeviation, float colorDeviation, float lifeDeviation, float sizeDeviation);
};

class ParticleEmitterManager {
public:
    ParticleEmitterManager(const RenderConfig& renderConfig);

    void render(GLuint renderBuffer, const glm::mat4& viewProjectionMatrix) const;
    void updateEmitters() { for (ParticleEmitter& emitter : emitters) { emitter.update(
        m_renderConfig.velocityDeviation, m_renderConfig.colorDeviation, m_renderConfig.lifeDeviation, m_renderConfig.sizeDeviation
    ); } }

    void addEmitter(glm::vec3 position)     { emitters.emplace_back(position,
                                                                    m_renderConfig.velocityDeviation,
                                                                    m_renderConfig.colorDeviation,
                                                                    m_renderConfig.lifeDeviation,
                                                                    m_renderConfig.sizeDeviation); }
    void removeEmitter(size_t idx)          { emitters.erase(emitters.begin() + idx); }
    ParticleEmitter& emitterAt(size_t idx)  { return emitters[idx]; }
    size_t numEmitters()                    { return emitters.size(); }


private:
    static constexpr GLuint INVALID = 0xFFFFFFFF;
    static constexpr std::array<GLfloat, 12UL> vertexBufferData = {
        -0.5f,  -0.5f,  0.0f,
        -0.5f,   0.5f,  0.0f,
         0.5f,   0.5f,  0.0f,
         0.5f,  -0.5f,  0.0f
    };

    std::vector<ParticleEmitter> emitters;

    // OpenGL rendering
    GLuint VAO          { INVALID };
    GLuint vertexVBO    { INVALID };
    GLuint particleVBO  { INVALID };
    Shader particleShader;

    const RenderConfig& m_renderConfig;

    void genAttributeBuffers();
    std::vector<ParticleShader> genSortedParticles(const glm::mat4& viewProjectionMatrix) const;
    void updateAndBindAttributeBuffers(const glm::mat4& viewProjectionMatrix) const;
};

#endif
