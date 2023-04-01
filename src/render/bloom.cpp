#include "bloom.h"

#include <iostream>
#include <utils/constants.h>
#include <utils/render_utils.hpp>

BloomFilter::BloomFilter(RenderConfig& renderConfig)
    : m_renderConfig(renderConfig) {
        initBrightBuffer();
        initBlurBuffers();
        initShaders();
}

BloomFilter::~BloomFilter() {
    glDeleteBuffers(1, &brightBuffer);
    glDeleteTextures(1, &brightTex);
    glDeleteBuffers(2, blurBuffers.data());
    glDeleteTextures(2, blurTextures.data());
}

GLuint BloomFilter::render(GLuint hdrTex, const float ensmall) {
    extractBrightRegions(hdrTex, ensmall);
    return computeBlur(ensmall);
}

void BloomFilter::initBrightBuffer() {
    // Create bright regions buffer
    glCreateFramebuffers(1, &brightBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, brightBuffer);

    // Create texture to render to
    glCreateTextures(GL_TEXTURE_2D, 1, &brightTex);
    glBindTexture(GL_TEXTURE_2D, brightTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, utils::WIDTH, utils::HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brightTex, 0);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::cerr << "Failed to initialise bloom bright regions framebuffer" << std::endl; }
    else { std::cout << "Initialized bloom bright regions framebuffer" << std::endl; }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BloomFilter::initBlurBuffers() {
    glCreateFramebuffers(2, blurBuffers.data());
    glCreateTextures(GL_TEXTURE_2D, 2, blurTextures.data());

    for (size_t blurIdx = 0UL; blurIdx < 2UL; blurIdx++) {
        glBindFramebuffer(GL_FRAMEBUFFER, blurBuffers[blurIdx]);
        glBindTexture(GL_TEXTURE_2D, blurTextures[blurIdx]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, utils::WIDTH, utils::HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTextures[blurIdx], 0);
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

void BloomFilter::extractBrightRegions(GLuint hdrTex, const float ensmall) {
    extractBright.bind();
    glBindFramebuffer(GL_FRAMEBUFFER, brightBuffer);
    glActiveTexture(GL_TEXTURE0 + utils::POST_PROCESSING_TEX_START_IDX);
    glBindTexture(GL_TEXTURE_2D, hdrTex);
    glUniform1i(0, utils::POST_PROCESSING_TEX_START_IDX);
    glUniform1f(1, m_renderConfig.bloomBrightThreshold);
    utils::renderQuad(ensmall);
}

GLuint BloomFilter::computeBlur(const float ensmall) {
    bool horizontal = true, firstIteration = true;
    gaussianBlur.bind();

    for (uint32_t iteration = 0; iteration < m_renderConfig.bloomIterations * 2U; iteration++) {
        glBindFramebuffer(GL_FRAMEBUFFER, blurBuffers[horizontal]);
        glActiveTexture(GL_TEXTURE0 + utils::POST_PROCESSING_TEX_START_IDX); 
        glBindTexture(GL_TEXTURE_2D, firstIteration ? brightTex : blurTextures[!horizontal]);
        glUniform1i(0, utils::POST_PROCESSING_TEX_START_IDX);
        glUniform1i(1, horizontal);
        utils::renderQuad(ensmall);

        horizontal      = !horizontal;
        firstIteration  = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return blurTextures[horizontal];
}
