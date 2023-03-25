#include "deferred.h"

DISABLE_WARNINGS_PUSH()
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()

#include <array>
#include <iostream>
#include <stdint.h>
#include <utils/constants.h>

DeferredRenderer::DeferredRenderer(RenderConfig& renderConfig, Scene& scene, LightManager& lightManager)
    : m_renderConfig(renderConfig)
    , m_scene(scene)
    , m_lightManager(lightManager) {
    initBuffers();
    initShaders();
}

DeferredRenderer::~DeferredRenderer() {
    glDeleteBuffers(1, &gBuffer);
    glDeleteTextures(1, &positionTex);
    glDeleteTextures(1, &normalTex);
    glDeleteTextures(1, &albedoTex);
    glDeleteRenderbuffers(1, &rboDepth);
}

void DeferredRenderer::render(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraPos) {
    glViewport(0, 0, utils::WIDTH, utils::HEIGHT);  // Set correct viewport size
    renderGeometry(viewProjectionMatrix);           // Geometry pass
    renderLighting(cameraPos);                      // Lighting pass
    renderHdr();                                    // HDR tonemapping and gamma correction
    copyDepthBuffer();                              // Copy G-buffer depth data to main framebuffer
}

void DeferredRenderer::initGBuffer() {
    // Init G-Buffer
    glCreateFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // Init attribute textures (keep textures at 4 values, e.g. RGBA, to avoid alignment issues)
    // and assign them as render targets
    glCreateTextures(GL_TEXTURE_2D, 1, &positionTex);
    glBindTexture(GL_TEXTURE_2D, positionTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, utils::WIDTH, utils::HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionTex, 0);
    glCreateTextures(GL_TEXTURE_2D, 1, &normalTex);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, utils::WIDTH, utils::HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex, 0);
    glCreateTextures(GL_TEXTURE_2D, 1, &albedoTex);
    glBindTexture(GL_TEXTURE_2D, albedoTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, utils::WIDTH, utils::HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedoTex, 0);
    std::array<GLuint, 3UL> attachments { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments.data());

    // Create and attach depth buffer
    glCreateRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, utils::WIDTH, utils::HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::cerr << "Failed to initialise deferred rendering framebuffer" << std::endl; }
    else { std::cout << "Initialized deferred rendering framebuffer" << std::endl; }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredRenderer::initHdrBuffer() {
    // Create HDR buffer
    glCreateFramebuffers(1, &hdrBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrBuffer);

    // Create texture to render to
    glCreateTextures(GL_TEXTURE_2D, 1, &hdrTex);
    glBindTexture(GL_TEXTURE_2D, hdrTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, utils::WIDTH, utils::HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTex, 0);

    // Create and attach depth buffer
    glCreateRenderbuffers(1, &hdrDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, hdrDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, utils::WIDTH, utils::HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrDepth);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::cerr << "Failed to initialise HDR intermediate framebuffer" << std::endl; }
    else { std::cout << "Initialized HDR intermediate framebuffer" << std::endl; }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredRenderer::initBuffers() {
    initGBuffer();
    initHdrBuffer();
}

void DeferredRenderer::initLightingShaders() {
    try {
        std::string diffuseFileName;
        switch (m_renderConfig.diffuseModel) {
            case DiffuseModel::Lambert:
                diffuseFileName = "lambert.frag";
                break;
            case DiffuseModel::ToonLambert:
                diffuseFileName = "toon_lambert.frag";
                break;
            case DiffuseModel::XToonLambert:
                diffuseFileName = "xtoon_lambert.frag";
                break;
        }
        ShaderBuilder lightingDiffuseBuilder;
        lightingDiffuseBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "lighting" / "deferred_lighting.vert");
        lightingDiffuseBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "lighting" / "diffuse" / diffuseFileName);
        lightingDiffuse = lightingDiffuseBuilder.build();

        std::string specularFileName;
        switch (m_renderConfig.specularModel) {
            case SpecularModel::Phong:
                specularFileName = "phong.frag";
                break;
            case SpecularModel::BlinnPhong:
                specularFileName = "blinn_phong.frag";
                break;
            case SpecularModel::ToonBlinnPhong:
                specularFileName = "toon_blinn_phong.frag";
                break;
            case SpecularModel::XToonBlinnPhong:
                specularFileName = "xtoon_blinn_phong.frag";
                break;
        }
        ShaderBuilder lightingSpecularBuilder;
        lightingSpecularBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "lighting" / "deferred_lighting.vert");
        lightingSpecularBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "lighting" / "specular" / specularFileName);
        lightingSpecular = lightingSpecularBuilder.build();
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
        hdrBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "hdr" / "hdr.vert");
        hdrBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "hdr" / "hdr.frag");
        hdrRender = hdrBuilder.build();
    } catch (ShaderLoadingException e) { std::cerr << e.what() << std::endl; }

    // Lighting pass
    initLightingShaders();
}

void DeferredRenderer::renderGeometry(const glm::mat4& viewProjectionMatrix) const {
    // Bind the G-buffer and clear its previously held values
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render each model
    for (size_t modelNum = 0U; modelNum < m_scene.numMeshes(); modelNum++) {
        const GPUMesh& mesh         = m_scene.meshAt(modelNum);
        const glm::mat4 modelMatrix = m_scene.modelMatrix(modelNum);

        // Normals should be transformed differently than positions (ignoring translations + dealing with scaling)
        // https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
        const glm::mat4 mvpMatrix           = viewProjectionMatrix * modelMatrix;
        const glm::mat3 normalModelMatrix   = glm::inverseTranspose(glm::mat3(modelMatrix));

        // Bind shader(s), light(s), and uniform(s)
        geometryPass.bind();
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
        // TODO: Properly process textures
        // if (mesh.hasTextureCoords()) {
        //     m_texture.bind(GL_TEXTURE0);
        //     glUniform1i(3, 0);
        //     glUniform1i(4, GL_TRUE);
        // } else { glUniform1i(4, GL_FALSE); }
        glUniform1i(4, GL_FALSE); // We don't actually have textures for now :)

        mesh.draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredRenderer::bindGBufferTextures() const {
    glActiveTexture(GL_TEXTURE0 + utils::G_BUFFER_START_IDX);
    glBindTexture(GL_TEXTURE_2D, positionTex);
    glUniform1i(0, utils::G_BUFFER_START_IDX);
    glActiveTexture(GL_TEXTURE0 + utils::G_BUFFER_START_IDX + 1);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glUniform1i(1, utils::G_BUFFER_START_IDX + 1);
    glActiveTexture(GL_TEXTURE0 + utils::G_BUFFER_START_IDX + 2);
    glBindTexture(GL_TEXTURE_2D, albedoTex);
    glUniform1i(2, utils::G_BUFFER_START_IDX + 2);
}

void DeferredRenderer::renderQuad() {
    if (quadVAO == 0U) {
        std::array<float, 20UL> quadVertices = {
            // Positions        // Texture coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };

        // Set up plane VAO
        glCreateVertexArrays(1, &quadVAO);
        glCreateBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void DeferredRenderer::renderDiffuse(const glm::vec3& cameraPos) {
    // Bind shader, G-buffer textures, and general usage uniforms
    lightingDiffuse.bind();
    bindGBufferTextures();
    glUniform3fv(3, 1, glm::value_ptr(cameraPos));
    glUniform1f(5, m_renderConfig.shadowFarPlane);
    m_lightManager.bind();

    // Bind shader-specific uniforms
    switch (m_renderConfig.diffuseModel) {
        case DiffuseModel::Lambert:
            break;
        case DiffuseModel::ToonLambert:
            glUniform1ui(8, m_renderConfig.toonDiscretizeSteps);
            break;
        case DiffuseModel::XToonLambert:
            // TODO: Add XToon texture
            break;
    }

    renderQuad();
}

void DeferredRenderer::renderSpecular(const glm::vec3& cameraPos) {
    // Bind shader, G-buffer textures, and general usage uniforms
    lightingSpecular.bind();
    bindGBufferTextures();
    glUniform3fv(3, 1, glm::value_ptr(cameraPos));
    glUniform1f(5, m_renderConfig.shadowFarPlane);
    m_lightManager.bind();

    // Bind shader-specific uniforms
    switch (m_renderConfig.specularModel) {
        case SpecularModel::Phong:
        case SpecularModel::BlinnPhong:
            break;
        case SpecularModel::ToonBlinnPhong:
            glUniform1f(8, m_renderConfig.toonSpecularThreshold);
            break;
        case SpecularModel::XToonBlinnPhong:
            // TODO: Add XToon texture
            break;
    }

    renderQuad();
}

void DeferredRenderer::renderLighting(const glm::vec3& cameraPos) {
    // Bind HDR framebuffer and clear previous values
    glBindFramebuffer(GL_FRAMEBUFFER, hdrBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // For diffuse and specular: bind lighting shader, G-Buffer data, and lighting data
    glDepthFunc(GL_ALWAYS);                     // Depth test always passes to allow for combining all fragment results
    glEnablei(GL_BLEND, hdrBuffer);             // Enable blending for main framebuffer
    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);    // Blend source and destination fragments based on their alpha components
    renderDiffuse(cameraPos);
    renderSpecular(cameraPos);
    glDisablei(GL_BLEND, hdrBuffer);            // Be a good citizen and restore defaults
    glDepthFunc(GL_LEQUAL);
}

void DeferredRenderer::renderHdr() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    hdrRender.bind();
    glActiveTexture(GL_TEXTURE0 + utils::HDR_BUFFER_START_IDX);
    glBindTexture(GL_TEXTURE_2D, hdrTex);
    glUniform1i(0, utils::HDR_BUFFER_START_IDX);
    glUniform1i(1, m_renderConfig.useHdr);
    glUniform1f(2, m_renderConfig.exposure);
    glUniform1f(3, m_renderConfig.gamma);
    renderQuad();
}

void DeferredRenderer::copyDepthBuffer() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
    glBlitFramebuffer(0, 0, utils::WIDTH, utils::HEIGHT, 0, 0, utils::WIDTH, utils::HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
