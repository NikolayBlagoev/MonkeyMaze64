#ifndef _DEFERRED_H_
#define _DEFERRED_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>

#include <render/bloom.h>
#include <render/config.h>
#include <render/lighting.h>
#include <render/particle.h>
#include <render/scene.h>
#include <render/texture.h>

// TODO: Adapt to resize framebuffer sizes when window size changes
class DeferredRenderer {
public:
    DeferredRenderer(int32_t renderWidth, int32_t renderHeight,
                     RenderConfig& renderConfig, Scene& scene, LightManager& lightManager,
                     ParticleEmitterManager& particleEmitterManager, std::weak_ptr<const Texture> xToonTex);
    ~DeferredRenderer();

    void render(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraPos);
    void initLightingShader();
    void copyGBufferDepth(GLuint destinationBuffer);

private:
    void initGBuffer();
    void initHdrBuffer();
    void initBuffers();
    void initShaders();

    void bindMaterialTextures(const GPUMesh& mesh, const glm::vec3& cameraPos) const;
    void helper(MeshTree* mt, const glm::mat4& currTransform, const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraPos) const;
    void renderGeometry(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraPos) const;
    
    void bindGBufferTextures() const;
    void renderLighting(const glm::vec3& cameraPos);

    void renderForward(const glm::mat4& viewProjectionMatrix);
    void renderPostProcessing();

    static constexpr GLuint INVALID     = 0xFFFFFFFF;
    static constexpr float clearDepth   = 1.0f;

    // Render resolution
    const int32_t RENDER_WIDTH  { utils::WIDTH };
    const int32_t RENDER_HEIGHT { utils::HEIGHT };

    GLuint gBuffer;     // Framebuffer ID for the G-buffer
    GLuint rboDepthG;   // G-buffer depth buffer

    // Textures for storing G-buffer attributes
    // Must match attributes used by fragment shader(s)
    GLuint positionTex;
    GLuint normalTex;
    GLuint albedoTex;
    GLuint materialTex; // Red channel is metallic, green channel is roughness, blue channel is AO

    // Intermediate HDR framebuffer to render to before tonemapping and gamma correction
    GLuint hdrBuffer;
    GLuint hdrTex;
    GLuint rboDepthHDR;

    // Shaders and shader-specific info
    Shader geometryPass;
    Shader lightingPass;
    Shader hdrRender;
    std::weak_ptr<const Texture> m_xToonTex;

    RenderConfig& m_renderConfig;
    Scene& m_scene;
    LightManager& m_lightManager;
    ParticleEmitterManager& m_particleEmitterManager;

    // Objects managing other rendering bits
    BloomFilter bloomFilter;
};

#endif
