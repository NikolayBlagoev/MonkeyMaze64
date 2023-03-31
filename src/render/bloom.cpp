#include "bloom.h"

#include <iostream>
#include <utils/constants.h>
#include <utils/render_utils.hpp>

BloomFilter::BloomFilter(RenderConfig& renderConfig)
    : m_renderConfig(renderConfig) {
        initBuffers();
        initTextures();
        initShaders();
}

BloomFilter::~BloomFilter() {
    glDeleteBuffers(1, &brightBuffer);
    glDeleteTextures(1, &brightTex);
    glDeleteBuffers(2, blurBuffers.data());
    glDeleteTextures(2, blurTextures.data());
}

GLuint BloomFilter::render(GLuint hdrTex) {
    extractBrightRegions(hdrTex);
    return computeBlur();
}

void BloomFilter::initBuffers() {
    glCreateFramebuffers(1, &brightBuffer);         // Bright regions
    glCreateFramebuffers(2, blurBuffers.data());    // Blur
}

void BloomFilter::initTextures() {
    // Bright regions
    glCreateTextures(GL_TEXTURE_2D, 1, &brightTex);
    glTextureStorage2D(brightTex, 1, GL_RGB16F, utils::WIDTH , utils::HEIGHT);
    glTextureParameteri(brightTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(brightTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(brightTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(brightTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glNamedFramebufferTexture(brightBuffer, GL_COLOR_ATTACHMENT0, brightTex, 0);
    if (glCheckNamedFramebufferStatus(brightBuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::cerr << "Failed to initialise bloom bright regions framebuffer" << std::endl; }

    // Blur
    glCreateTextures(GL_TEXTURE_2D, 2, blurTextures.data());
    for (size_t blurIdx = 0UL; blurIdx < 2UL; blurIdx++) {
        glTextureStorage2D(blurTextures[blurIdx], 1, GL_RGB16F, utils::WIDTH , utils::HEIGHT);
        glTextureParameteri(blurTextures[blurIdx], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(blurTextures[blurIdx], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(blurTextures[blurIdx], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(blurTextures[blurIdx], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glNamedFramebufferTexture(blurBuffers[blurIdx], GL_COLOR_ATTACHMENT0, blurTextures[blurIdx], 0);
        if (glCheckNamedFramebufferStatus(blurBuffers[blurIdx], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::cerr << "Failed to initialise bloom blur framebuffer " << blurIdx << std::endl; }
    }
}

void BloomFilter::initShaders() {
    try {
        ShaderBuilder extractBrightBuilder;
        extractBrightBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "screen-quad.vert");
        extractBrightBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "post-processing" / "bloom" / "extract-bright.frag");
        extractBright = extractBrightBuilder.build();

        ShaderBuilder gaussianBlurBuilder;
        gaussianBlurBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "screen-quad.vert");
        gaussianBlurBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "post-processing" / "bloom" / "gaussian-blur.frag");
        gaussianBlur = gaussianBlurBuilder.build();
    } catch (ShaderLoadingException e) { std::cerr << e.what() << std::endl; }
}

void BloomFilter::extractBrightRegions(GLuint hdrTex) {
    // Bind brightness extractness shader and bright regions framebuffer
    extractBright.bind();
    glBindFramebuffer(GL_FRAMEBUFFER, brightBuffer);

    // Bind HDR texture
    glActiveTexture(GL_TEXTURE0 + utils::POST_PROCESSING_TEX_START_IDX);
    glBindTexture(GL_TEXTURE_2D, hdrTex);

    // Set uniforms and render
    glUniform1i(0, utils::POST_PROCESSING_TEX_START_IDX);
    glUniform1f(1, m_renderConfig.bloomBrightThreshold);
    utils::renderQuad();
}

GLuint BloomFilter::computeBlur() {
    bool horizontal = true, firstIteration = true;
    gaussianBlur.bind();

    for (uint32_t iteration = 0; iteration < m_renderConfig.bloomIterations * 2U; iteration++) {
        glBindFramebuffer(GL_FRAMEBUFFER, blurBuffers[horizontal]);
        glActiveTexture(GL_TEXTURE0 + utils::POST_PROCESSING_TEX_START_IDX); 
        glBindTexture(GL_TEXTURE_2D, firstIteration ? brightTex : blurTextures[!horizontal]);
        glUniform1i(0, utils::POST_PROCESSING_TEX_START_IDX);
        glUniform1i(1, horizontal);
        utils::renderQuad();

        horizontal      = !horizontal;
        firstIteration  = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return blurTextures[horizontal];
}
