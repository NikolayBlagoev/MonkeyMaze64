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
    BloomFilter(const RenderConfig& renderConfig);
    ~BloomFilter();

    GLuint render(GLuint hdrTex);

private:
    void initBuffers();
    void initTextures();
    void initShaders();

    // Render computations
    void extractBrightRegions(GLuint hdrTex);
    GLuint computeBlur();

    // Framebuffer, shader and texture for extracting bright regions of rendered image
    GLuint brightBuffer;
    GLuint brightTex;
    Shader extractBright;

    // Framebuffer, shader and texture for ping-pong gaussian blur computation
    std::array<GLuint, 2UL> blurBuffers;
    std::array<GLuint, 2UL> blurTextures;
    Shader gaussianBlur;

    const RenderConfig& m_renderConfig;
};

#endif
