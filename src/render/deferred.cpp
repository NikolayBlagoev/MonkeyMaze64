#include "deferred.h"

DISABLE_WARNINGS_PUSH()
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()

#include <array>
#include <iostream>
#include <stdint.h>
#include <utils/constants.h>

DeferredRenderer::DeferredRenderer(RenderConfig& renderConfig, Scene& scene, LightManager& lightManager,
                                   int32_t screenWidth, int32_t screenHeight)
    : m_renderConfig(renderConfig)
    , m_scene(scene)
    , m_lightManager(lightManager) {
    initBuffers(screenWidth, screenHeight);
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
    // Set correct viewport size
    glViewport(0, 0, utils::WIDTH, utils::HEIGHT);

    // Geometry pass
    renderGeometry(viewProjectionMatrix);

    // Bind screen framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Bind lighting shader, G-Buffer data, and lighting data
    lightingPass.bind();
    bindGBufferTextures();
    glUniform3fv(3, 1, glm::value_ptr(cameraPos));
    glUniform1f(5, m_renderConfig.shadowFarPlane);
    m_lightManager.bind();

    // Draw final screen quad (lighting pass)
    renderQuad();
}

void DeferredRenderer::initBuffers(int32_t screenWidth, int32_t screenHeight) {
    // Init G-Buffer
    glCreateFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // Init attribute textures (keep textures at 4 values, e.g. RGBA, to avoid alignment issues)
    // and assign them as render targets
    glCreateTextures(GL_TEXTURE_2D, 1, &positionTex);
    glBindTexture(GL_TEXTURE_2D, positionTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionTex, 0);
    glCreateTextures(GL_TEXTURE_2D, 1, &normalTex);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex, 0);
    glCreateTextures(GL_TEXTURE_2D, 1, &albedoTex);
    glBindTexture(GL_TEXTURE_2D, albedoTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedoTex, 0);
    std::array<GLuint, 3UL> attachments { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments.data());

    // Create and attach depth buffer
    glCreateRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::cerr << "Failed to initialise deferred rendering framebuffer" << std::endl; }
    else { std::cout << "Initialized deferred rendering framebuffer" << std::endl; }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredRenderer::initShaders() {
    try {
        ShaderBuilder geometryPassBuilder;
        geometryPassBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "deferred" / "deferred.vert");
        geometryPassBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "deferred" / "deferred.frag");
        geometryPass = geometryPassBuilder.build();

        ShaderBuilder lightingPassBuilder;
        lightingPassBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "lighting" / "deferred_lighting.vert");
        lightingPassBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "lighting" / "phong.frag");
        lightingPass = lightingPassBuilder.build();
    } catch (ShaderLoadingException e) { std::cerr << e.what() << std::endl; }
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
    glBindVertexArray(0);
}
