#ifndef _DEFERRED_H_
#define _DEFERRED_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>

#include <render/config.h>
#include <render/lighting.h>
#include <render/scene.h>

// TODO: Adapt to resize framebuffer sizes when window size changes
class DeferredRenderer {
public:
    DeferredRenderer(RenderConfig& renderConfig, Scene& scene, LightManager& lightManager);
    ~DeferredRenderer();

    void render(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraPos);
    void initLightingShaders();

private:
    void initGBuffer();
    void initHdrBuffer();
    void initBuffers();
    void initShaders();
    void renderGeometry(const glm::mat4& viewProjectionMatrix) const;
    void bindGBufferTextures() const;
    void renderQuad();
    void renderDiffuse(const glm::vec3& cameraPos);
    void renderSpecular(const glm::vec3& cameraPos);
    void renderLighting(const glm::vec3& cameraPos);
    void renderHdr();
    void copyDepthBuffer();

    static constexpr GLuint INVALID = 0xFFFFFFFF;

    GLuint gBuffer;     // Framebuffer ID for the G-buffer
    GLuint rboDepth;    // Depth buffer

    // Quad to render final lighting to
    GLuint quadVAO = 0U;
    GLuint quadVBO;

    // Textures for storing G-buffer attributes
    // Must match attributes used by fragment shader(s)
    GLuint positionTex;
    GLuint normalTex;
    GLuint albedoTex;

    // Intermediate HDR framebuffer to render to before tonemapping and gamma correction
    GLuint hdrBuffer;
    GLuint hdrTex;
    GLuint hdrDepth;

    Shader geometryPass;
    Shader lightingDiffuse;
    Shader lightingSpecular;
    Shader hdrRender;

    RenderConfig& m_renderConfig;
    Scene& m_scene;
    LightManager& m_lightManager;
};

#endif
