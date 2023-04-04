#ifndef _SSAO_H_
#define _SSAO_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>

#include <render/config.h>
#include <utils/misc_utils.hpp>

class SSAOFilter {
public:
    SSAOFilter(int32_t renderWidth, int32_t renderHeight, RenderConfig& renderConfig);
    ~SSAOFilter();

    GLuint render(GLuint positionTex, GLuint normalTex, const glm::mat4& viewProjection) const;
    void regenSamples() { generateSamples(); }
    void regenRandomRotation();

private:
    void initRender();
    void initBlur();
    void createRandomRotationTex();
    void initSampling();
    void generateSamples();
    void generateRandomRotation();

    // Render computations
    void computeSSAO(GLuint positionTex, GLuint normalTex, const glm::mat4& viewProjection) const;
    GLuint computeBlur() const;

    // Render resolution
    const int32_t RENDER_WIDTH  { utils::WIDTH };
    const int32_t RENDER_HEIGHT { utils::HEIGHT };

    // Framebuffer, shader and texture for computing SSAO effect
    GLuint renderBuffer;
    GLuint renderTex;
    Shader ssaoRender;

    // Framebuffer, shader and texture for blurring SSAO samples
    GLuint blurBuffer;
    GLuint blurTex;
    Shader ssaoBlur;

    // Sample and noise data (fourth element is unused and only there to avoid alignment issues)
    GLuint ssboSamples;
    std::vector<glm::vec4> samplesData;
    GLuint randomRotationTex;
    std::vector<glm::vec4> randomRotationData;

    const RenderConfig& m_renderConfig;
};

#endif
