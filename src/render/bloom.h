#ifndef _BLOOM_H_
#define _BLOOM_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>

#include <array>
#include <render/config.h>

class BloomFilter {
public:
    BloomFilter(RenderConfig& renderConfig);
    ~BloomFilter();

    GLuint render(GLuint hdrTex, const float ensmall);

private:
    void initBrightBuffer();
    void initBlurBuffers();
    void initShaders();
    void extractBrightRegions(GLuint hdrTex, const float ensmall);
    GLuint computeBlur(const float ensmall);

    // Framebuffer (and corresponding texture) for extracting bright regions of rendered image
    GLuint brightBuffer;
    GLuint brightTex;

    // Framebuffer (and corresponding texture) pair for ping-pong gaussian blur computation
    std::array<GLuint, 2UL> blurBuffers;
    std::array<GLuint, 2UL> blurTextures;

    Shader extractBright;
    Shader gaussianBlur;

    RenderConfig& m_renderConfig;
};

#endif
