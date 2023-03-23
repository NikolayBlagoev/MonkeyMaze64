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
    DeferredRenderer(RenderConfig& renderConfig, Scene& scene, LightManager& lightManager,
                     int32_t screenWidth, int32_t screenHeight);
    ~DeferredRenderer();

    void render(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraPos);

private:
    void initBuffers(int32_t screenWidth, int32_t screenHeight);
    void initShaders();
    void renderGeometry(const glm::mat4& viewProjectionMatrix) const;
    void bindGBufferTextures() const;
    void renderQuad();

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

    Shader geometryPass;
    Shader lightingPass;

    RenderConfig& m_renderConfig;
    Scene& m_scene;
    LightManager& m_lightManager;
};

#endif
