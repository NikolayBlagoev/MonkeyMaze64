#include "ssao.h"

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()

#include <utils/render_utils.hpp>
#include <iostream>

SSAOFilter::SSAOFilter(int32_t renderWidth, int32_t renderHeight, RenderConfig& renderConfig)
: RENDER_WIDTH(renderWidth)
, RENDER_HEIGHT(renderHeight)
, m_renderConfig(renderConfig) {
    initRender();
    initBlur();
    initSampling();
    generateSamples();
    generateRandomRotation();
}

SSAOFilter::~SSAOFilter() {
    glDeleteBuffers(1, &renderBuffer);
    glDeleteTextures(1, &renderTex);
    glDeleteBuffers(1, &blurBuffer);
    glDeleteTextures(1, &blurTex);
    glDeleteBuffers(1, &ssboSamples);
    glDeleteTextures(1, &randomRotationTex);
}

GLuint SSAOFilter::render(GLuint positionTex, GLuint normalTex, const glm::mat4& viewProjection) const {
    computeSSAO(positionTex, normalTex, viewProjection);
    return computeBlur();
}

void SSAOFilter::regenRandomRotation() {
    // Destroy old texture then create a new one and populate it with data
    glDeleteTextures(1, &randomRotationTex);
    createRandomRotationTex();
    generateRandomRotation();
}

void SSAOFilter::initRender() {
    glCreateFramebuffers(1, &renderBuffer);

    glCreateTextures(GL_TEXTURE_2D, 1, &renderTex);
    glTextureStorage2D(renderTex, 1, GL_R16F, RENDER_WIDTH , RENDER_HEIGHT); // Only red channel as we are dealing with just depth values
    glTextureParameteri(renderTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(renderTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(renderTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(renderTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glNamedFramebufferTexture(renderBuffer, GL_COLOR_ATTACHMENT0, renderTex, 0);
    if (glCheckNamedFramebufferStatus(renderBuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::cerr << "Failed to initialise SSAO main render framebuffer" << std::endl; }

    try {
        ShaderBuilder ssaoRenderBuilder;
        ssaoRenderBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "screen-quad.vert");
        ssaoRenderBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "post-processing" / "ssao" / "main_render.frag");
        ssaoRender = ssaoRenderBuilder.build();
    } catch (ShaderLoadingException e) { std::cerr << e.what() << std::endl; }
}

void SSAOFilter::initBlur() {
    glCreateFramebuffers(1, &blurBuffer);

    glCreateTextures(GL_TEXTURE_2D, 1, &blurTex);
    glTextureStorage2D(blurTex, 1, GL_R16F, RENDER_WIDTH , RENDER_HEIGHT); // Only red channel as we are dealing with just depth values
    glTextureParameteri(blurTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(blurTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(blurTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(blurTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glNamedFramebufferTexture(blurBuffer, GL_COLOR_ATTACHMENT0, blurTex, 0);
    if (glCheckNamedFramebufferStatus(blurBuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::cerr << "Failed to initialise SSAO blur framebuffer" << std::endl; }

    try {
        ShaderBuilder ssaoBlurBuilder;
        ssaoBlurBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "screen-quad.vert");
        ssaoBlurBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "post-processing" / "ssao" / "blur.frag");
        ssaoBlur = ssaoBlurBuilder.build();
    } catch (ShaderLoadingException e) { std::cerr << e.what() << std::endl; }
}

void SSAOFilter::createRandomRotationTex() {
    glCreateTextures(GL_TEXTURE_2D, 1, &randomRotationTex);
    glTextureStorage2D(randomRotationTex, 1, GL_RGBA32F, static_cast<GLsizei>(m_renderConfig.ssaoKernelLength), static_cast<GLsizei>(m_renderConfig.ssaoKernelLength));
    glTextureParameteri(randomRotationTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(randomRotationTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(randomRotationTex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(randomRotationTex, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void SSAOFilter::initSampling() {
    glCreateBuffers(1, &ssboSamples);
    createRandomRotationTex();
}

void SSAOFilter::generateSamples() {
    samplesData = utils::randomVectors(glm::vec2(-1.0f, 1.0f), glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 1.0f), m_renderConfig.ssaoSamples);
    glNamedBufferData(ssboSamples, sizeof(glm::vec4) * samplesData.size(), samplesData.data(), GL_STATIC_COPY);
}

void SSAOFilter::generateRandomRotation() {
    randomRotationData = utils::randomVectors(glm::vec2(-1.0f, 1.0f), glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f),
                                              m_renderConfig.ssaoKernelLength * m_renderConfig.ssaoKernelLength,
                                              false, false);
    glTextureSubImage2D(randomRotationTex, 0,
                        0, 0, static_cast<GLsizei>(m_renderConfig.ssaoKernelLength), static_cast<GLsizei>(m_renderConfig.ssaoKernelLength),
                        GL_RGBA, GL_FLOAT, randomRotationData.data());
}

void SSAOFilter::computeSSAO(GLuint positionTex, GLuint normalTex, const glm::mat4& viewProjection) const {
    // Bind SSAO computation shader and bright regions framebuffer
    ssaoRender.bind();
    glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer);

    // Bind sample vectors SSBO to binding=0
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboSamples);

    // Bind G-buffer and random rotation textures
    glActiveTexture(GL_TEXTURE0 + utils::POST_PROCESSING_TEX_START_IDX);
    glBindTexture(GL_TEXTURE_2D, positionTex);
    glUniform1i(0, utils::POST_PROCESSING_TEX_START_IDX);
    glActiveTexture(GL_TEXTURE0 + utils::POST_PROCESSING_TEX_START_IDX + 1);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glUniform1i(1, utils::POST_PROCESSING_TEX_START_IDX + 1);
    glActiveTexture(GL_TEXTURE0 + utils::POST_PROCESSING_TEX_START_IDX + 2);
    glBindTexture(GL_TEXTURE_2D, randomRotationTex);
    glUniform1i(2, utils::POST_PROCESSING_TEX_START_IDX + 2);

    // Set remaining uniforms (see shaders/ssao/main_render.frag for details)
    glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(viewProjection));
    glUniform2f(4, static_cast<float>(RENDER_WIDTH), static_cast<float>(RENDER_HEIGHT));
    glUniform1f(5, static_cast<float>(m_renderConfig.ssaoKernelLength));
    glUniform1f(6, m_renderConfig.ssaoRadius);
    glUniform1f(7, m_renderConfig.ssaoBias);
    glUniform1f(8, m_renderConfig.ssaoPower);

    utils::renderQuad();
}

GLuint SSAOFilter::computeBlur() const {
    // Bind blur shader and corresponding framebuffer
    ssaoBlur.bind();
    glBindFramebuffer(GL_FRAMEBUFFER, blurBuffer);

    // Bind main SSAO computation result texture
    glActiveTexture(GL_TEXTURE0 + utils::POST_PROCESSING_TEX_START_IDX);
    glBindTexture(GL_TEXTURE_2D, renderTex);
    glUniform1i(0, utils::POST_PROCESSING_TEX_START_IDX);

    // Set uniform(s) and render
    glUniform1i(1, static_cast<int32_t>(m_renderConfig.ssaoKernelLength));
    utils::renderQuad();

    return blurTex;
}
