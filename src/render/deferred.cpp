#include "deferred.h"

DISABLE_WARNINGS_PUSH()
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()

#include <array>
#include <iostream>
#include <stdint.h>
#include <utils/constants.h>
#include <utils/render_utils.hpp>

DeferredRenderer::DeferredRenderer(int32_t renderWidth, int32_t renderHeight,
                                   RenderConfig& renderConfig, Scene& scene, LightManager& lightManager,
                                   ParticleEmitterManager& particleEmitterManager, std::weak_ptr<const Texture> xToonTex)
    : RENDER_WIDTH(renderWidth)
    , RENDER_HEIGHT(renderHeight)
    , m_renderConfig(renderConfig)
    , m_scene(scene)
    , m_lightManager(lightManager)
    , m_particleEmitterManager(particleEmitterManager)
    , m_xToonTex(xToonTex)
    , bloomFilter(renderWidth, renderHeight, renderConfig) {
    , ssaoFilter(renderWidth, renderHeight, renderConfig) {
    initBuffers();
    initShaders();
}

DeferredRenderer::~DeferredRenderer() {
    // G-buffer
    glDeleteBuffers(1, &gBuffer);
    glDeleteTextures(1, &positionTex);
    glDeleteTextures(1, &normalTex);
    glDeleteTextures(1, &albedoTex);
    glDeleteTextures(1, &materialTex);
    glDeleteRenderbuffers(1, &rboDepthG);

    // HDR buffer
    glDeleteBuffers(1, &hdrBuffer);
    glDeleteTextures(1, &hdrTex);
}

void DeferredRenderer::render(const glm::mat4& viewProjection, const glm::vec3& cameraPos) {
    glViewport(0, 0, RENDER_WIDTH, RENDER_HEIGHT);  // Set correct viewport size
    renderGeometry(viewProjection, cameraPos);      // Geometry pass
    renderLighting(cameraPos);                      // Lighting pass
    copyGBufferDepth(hdrBuffer);                    // Copy G-buffer depth data to HDR framebuffer for use with forward rendering
    renderForward(viewProjection);                  // Render transparent objects which require forward rendering
    renderPostProcessing(viewProjection);           // Combine post-processing results; HDR tonemapping and gamma correction
    copyGBufferDepth(0U);                           // Copy G-buffer depth data to main framebuffer for 3D UI elements rendering
}

void DeferredRenderer::initGBuffer() {
    // Init G-Buffer
    glCreateFramebuffers(1, &gBuffer);

    // Init attribute textures (keep textures at 4 values, e.g. RGBA, to avoid alignment issues)
    // and assign them as render targets
    glCreateTextures(GL_TEXTURE_2D, 1, &positionTex);
    glTextureStorage2D(positionTex, 1, GL_RGBA16F, RENDER_WIDTH , RENDER_HEIGHT);
    glTextureParameteri(positionTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(positionTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT0, positionTex, 0);
    glCreateTextures(GL_TEXTURE_2D, 1, &normalTex);
    glTextureStorage2D(normalTex, 1, GL_RGBA16F, RENDER_WIDTH , RENDER_HEIGHT);
    glTextureParameteri(normalTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(normalTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT1, normalTex, 0);
    glCreateTextures(GL_TEXTURE_2D, 1, &albedoTex);
    glTextureStorage2D(albedoTex, 1, GL_RGBA8, RENDER_WIDTH, RENDER_HEIGHT);
    glTextureParameteri(albedoTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(albedoTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT2, albedoTex, 0);
    glCreateTextures(GL_TEXTURE_2D, 1, &materialTex);
    glTextureStorage2D(materialTex, 1, GL_RGBA8, RENDER_WIDTH, RENDER_HEIGHT);
    glTextureParameteri(materialTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(materialTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT3, materialTex, 0);
    std::array<GLuint, 4UL> attachments { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glNamedFramebufferDrawBuffers(gBuffer, 4, attachments.data());

    // Create and attach depth buffer
    glCreateRenderbuffers(1, &rboDepthG);
    glNamedRenderbufferStorage(rboDepthG, GL_DEPTH_COMPONENT, RENDER_WIDTH, RENDER_HEIGHT);
    glNamedFramebufferRenderbuffer(gBuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthG);

    // Check if framebuffer is complete
    if (glCheckNamedFramebufferStatus(gBuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::cerr << "Failed to initialise deferred rendering framebuffer" << std::endl; }
}

void DeferredRenderer::initHdrBuffer() {
    // Create HDR buffer
    glCreateFramebuffers(1, &hdrBuffer);

    // Create texture to render to
    glCreateTextures(GL_TEXTURE_2D, 1, &hdrTex);
    glTextureStorage2D(hdrTex, 1, GL_RGBA16F, RENDER_WIDTH, RENDER_HEIGHT);
    glTextureParameteri(hdrTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(hdrTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(hdrBuffer, GL_COLOR_ATTACHMENT0, hdrTex, 0);

    // Create and attach depth buffer
    glCreateRenderbuffers(1, &rboDepthHDR);
    glNamedRenderbufferStorage(rboDepthHDR, GL_DEPTH_COMPONENT, RENDER_WIDTH, RENDER_HEIGHT);
    glNamedFramebufferRenderbuffer(hdrBuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthHDR);

    // Check if framebuffer is complete
    if (glCheckNamedFramebufferStatus(hdrBuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::cerr << "Failed to initialise HDR intermediate framebuffer" << std::endl; }
}

void DeferredRenderer::initBuffers() {
    initGBuffer();
    initHdrBuffer();
}

void DeferredRenderer::initLightingShader() {
    try {
        std::string lightingFileName;
        switch (m_renderConfig.lightingModel) {
            case LightingModel::LambertPhong:
                lightingFileName = "lambert_phong.frag";
                break;
            case LightingModel::LambertBlinnPhong:
                lightingFileName = "lambert_blinn_phong.frag";
                break;
            case LightingModel::Toon:
                lightingFileName = "toon.frag";
                break;
            case LightingModel::XToon:
                lightingFileName = "xtoon.frag";
                break;
            case LightingModel::PBR:
                lightingFileName = "pbr.frag";
                break;
        }
        ShaderBuilder lightingBuilder;
        lightingBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "screen-quad.vert");;
        lightingBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "lighting" / lightingFileName);
        lightingPass = lightingBuilder.build();
    } catch (ShaderLoadingException e) { std::cerr << e.what() << std::endl; }
}

void DeferredRenderer::initShaders() {
    // Geometry pass and HDR rendering
    try {
        ShaderBuilder geometryPassBuilder;
        geometryPassBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "deferred" / "deferred.vert");
        geometryPassBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "deferred" / "deferred.frag");
        geometryPass = geometryPassBuilder.build();

        ShaderBuilder hdrBuilder;
        hdrBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "screen-quad.vert");
        hdrBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "hdr.frag");
        hdrRender = hdrBuilder.build();
    } catch (ShaderLoadingException e) { std::cerr << e.what() << std::endl; }

    // Lighting pass
    initLightingShader();
}

void DeferredRenderer::bindMaterialTextures(const GPUMesh& mesh, const glm::vec3& cameraPos) const {
    // Albedo
    if (!mesh.getAlbedo().expired()) {
        mesh.getAlbedo().lock()->bind(GL_TEXTURE0);
        glUniform1i(3, 0);
    }
    glUniform1i(4, !mesh.getNormal().expired());
    glUniform4fv(5, 1, glm::value_ptr(m_renderConfig.defaultAlbedo));

    // Normal
    if (!mesh.getNormal().expired()) {
        mesh.getNormal().lock()->bind(GL_TEXTURE0 + 1);
        glUniform1i(6, 1);
    }
    glUniform1i(7, !mesh.getNormal().expired());

    // Metallic
    if (!mesh.getMetallic().expired()) {
        mesh.getMetallic().lock()->bind(GL_TEXTURE0 + 2);
        glUniform1i(8, 2);
    }
    glUniform1i(9, !mesh.getMetallic().expired());
    glUniform1f(10, m_renderConfig.defaultMetallic);

    // Roughness
    if (!mesh.getRoughness().expired()) {
        mesh.getRoughness().lock()->bind(GL_TEXTURE0 + 3);
        glUniform1i(11, 3);
    }
    glUniform1i(12, !mesh.getRoughness().expired());
    glUniform1f(13, m_renderConfig.defaultRoughness);

    // AO
    if (!mesh.getAO().expired()) {
        mesh.getAO().lock()->bind(GL_TEXTURE0 + 4);
        glUniform1i(14, 4);
    }
    glUniform1i(15, !mesh.getAO().expired());
    glUniform1f(16, m_renderConfig.defaultAO);

    // Displacement
    if (!mesh.getDisplacement().expired()) {
        mesh.getDisplacement().lock()->bind(GL_TEXTURE0 + 5);
        glUniform1i(17, 5);
    }
    glUniform1i(18, !mesh.getDisplacement().expired());
    glUniform1i(19, mesh.getIsHeight());
    glUniform1f(20, m_renderConfig.heightScale);
    glUniform1f(21, m_renderConfig.minDepthLayers);
    glUniform1f(22, m_renderConfig.maxDepthLayers);
    
    // Camera position
    glUniform3fv(23, 1, glm::value_ptr(cameraPos));
}


void DeferredRenderer::recursiveGeometryRender(MeshTree* mt, const glm::mat4& viewProjection, const glm::vec3& cameraPos) const {
    if (mt == nullptr) { return; }
   
    const glm::mat4& modelMatrix = mt->modelMatrix();
    if (mt->mesh != nullptr) {
        const GPUMesh& mesh = *(mt->mesh);
        
        // Normals should be transformed differently than positions (ignoring translations + dealing with scaling)
        // https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
        const glm::mat4 mvpMatrix           = viewProjection * modelMatrix;
        const glm::mat3 normalModelMatrix   = glm::inverseTranspose(glm::mat3(modelMatrix));

        // Bind shader program, transformation matrices, and material property textures
        geometryPass.bind();
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
        bindMaterialTextures(mesh, cameraPos);

        mesh.draw();   
    }
    
    for (size_t childIdx = 0; childIdx < mt->children.size(); childIdx++) {
        std::weak_ptr<MeshTree> childNode = mt->children.at(childIdx);
        if (childNode.expired()) {
            mt->children.erase(mt->children.begin() + childIdx);
            childIdx--;
            continue;
        }
        std::shared_ptr<MeshTree> childLock = childNode.lock();
        recursiveGeometryRender(childLock.get(), viewProjection, cameraPos);
    }
}

void DeferredRenderer::renderGeometry(const glm::mat4& viewProjection, const glm::vec3& cameraPos) const {
    // Clear color and depth values then render each model
    glClearTexImage(positionTex,    0, GL_RGBA, GL_HALF_FLOAT, 0);
    glClearTexImage(normalTex,      0, GL_RGBA, GL_HALF_FLOAT, 0);
    glClearTexImage(albedoTex,      0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glClearTexImage(materialTex,    0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glClearNamedFramebufferfv(gBuffer, GL_DEPTH, 0, &clearDepth);

    // Bind G-Buffer and render each model
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    recursiveGeometryRender(m_scene.root, viewProjection, cameraPos);
}

void DeferredRenderer::bindGBufferTextures() const {
    glActiveTexture(GL_TEXTURE0 + utils::G_BUFFER_TEX_START_IDX);
    glBindTexture(GL_TEXTURE_2D, positionTex);
    glUniform1i(0, utils::G_BUFFER_TEX_START_IDX);
    glActiveTexture(GL_TEXTURE0 + utils::G_BUFFER_TEX_START_IDX + 1);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glUniform1i(1, utils::G_BUFFER_TEX_START_IDX + 1);
    glActiveTexture(GL_TEXTURE0 + utils::G_BUFFER_TEX_START_IDX + 2);
    glBindTexture(GL_TEXTURE_2D, albedoTex);
    glUniform1i(2, utils::G_BUFFER_TEX_START_IDX + 2);
    glActiveTexture(GL_TEXTURE0 + utils::G_BUFFER_TEX_START_IDX + 3);
    glBindTexture(GL_TEXTURE_2D, materialTex);
    glUniform1i(3, utils::G_BUFFER_TEX_START_IDX + 3);
}

void DeferredRenderer::renderLighting(const glm::vec3& cameraPos) {
    // Clear previous HDR texture values
    glClearTexImage(hdrTex, 0, GL_RGBA, GL_HALF_FLOAT, 0);
    glClearNamedFramebufferfv(hdrBuffer, GL_DEPTH, 0, &clearDepth);
    
    // Bind HDR framebuffer, lighting shader, G-buffer textures, and set general usage uniforms
    glBindFramebuffer(GL_FRAMEBUFFER, hdrBuffer);
    lightingPass.bind();
    bindGBufferTextures();
    glUniform3fv(4, 1, glm::value_ptr(cameraPos));
    glUniform1f(6, m_renderConfig.shadowFarPlane);
    m_lightManager.bind();

    // Bind shader-specific uniforms
    switch (m_renderConfig.lightingModel) {
        case LightingModel::LambertPhong:
        case LightingModel::LambertBlinnPhong:
        case LightingModel::PBR:
            break;
        case LightingModel::Toon:
            glUniform1ui(9, m_renderConfig.toonDiscretizeSteps);
            glUniform1f(10, m_renderConfig.toonSpecularThreshold);
            break;
        case LightingModel::XToon:
            m_xToonTex.lock()->bind(GL_TEXTURE0 + utils::LIGHTING_TEX_START_IDX);
            glUniform1i(9, utils::LIGHTING_TEX_START_IDX);
            break;
    }

    utils::renderQuad();
}

void DeferredRenderer::renderForward(const glm::mat4& viewProjection) {
    m_particleEmitterManager.render(hdrBuffer, viewProjection); // Only particles need forward shading for now
}

void DeferredRenderer::renderPostProcessing(const glm::mat4& viewProjection) {
    // Render post-processing effect(s)
    GLuint bloomTex, ssaoTex;
    if (m_renderConfig.enableBloom) { bloomTex  = bloomFilter.render(hdrTex); }
    if (m_renderConfig.enableSSAO)  { ssaoTex   = ssaoFilter.render(positionTex, normalTex, viewProjection); }
    
    // Bind core HDR and tonemapping data
    hdrRender.bind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0 + utils::HDR_BUFFER_TEX_START_IDX);
    glBindTexture(GL_TEXTURE_2D, hdrTex);
    glUniform1i(0, utils::HDR_BUFFER_TEX_START_IDX);
    glUniform1i(1, m_renderConfig.enableHdr);
    glUniform1f(2, m_renderConfig.exposure);
    glUniform1f(3, m_renderConfig.gamma);

    // Bind bloom data
    if (m_renderConfig.enableBloom) {
        glActiveTexture(GL_TEXTURE0 + utils::HDR_BUFFER_TEX_START_IDX + 1);
        glBindTexture(GL_TEXTURE_2D, bloomTex);
        glUniform1i(4, utils::HDR_BUFFER_TEX_START_IDX + 1);
    }
    glUniform1i(5, m_renderConfig.enableBloom);

    // Bind SSAO data
    if (m_renderConfig.enableSSAO) {
        glActiveTexture(GL_TEXTURE0 + utils::HDR_BUFFER_TEX_START_IDX + 2);
        glBindTexture(GL_TEXTURE_2D, ssaoTex);
        glUniform1i(6, utils::HDR_BUFFER_TEX_START_IDX + 2);
        glUniform1f(7, m_renderConfig.ssaoOcclussionCoefficient);
    }
    glUniform1i(8, m_renderConfig.enableSSAO);
    
    glViewport(0, 0, RENDER_WIDTH, RENDER_HEIGHT);
    utils::renderQuad();
}

void DeferredRenderer::copyGBufferDepth(GLuint destinationBuffer) {
    glBlitNamedFramebuffer(gBuffer, destinationBuffer,
                           0, 0, RENDER_WIDTH, RENDER_HEIGHT,
                           0, 0, RENDER_WIDTH, RENDER_HEIGHT,
                           GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
