#ifndef _RENDER_UTILS_H_
#define _RENDER_UTILS_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()

#include <render/lighting.h>
#include <render/mesh_tree.h>
#include <render/mesh.h>
#include <iostream>

namespace utils {
    /********** Screen-filling quad to render to **********/ 
    static GLuint quadVAO = 0U;
    static GLuint quadVBO;
    static void renderQuad() {
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

    /********** Shadow map rendering **********/ 
    static Shader pointShadowMapsComp;
    static Shader areaShadowMapsComp;
    static void initShadowShaders() {
        try {
            ShaderBuilder pointShadowBuilder;
            pointShadowBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "shadows" / "shadow_point.vert");
            pointShadowBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH / "shadows" / "shadow_point.frag");
            pointShadowMapsComp = pointShadowBuilder.build();

            ShaderBuilder areaShadowBuilder;
            areaShadowBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "shadows" / "shadow_stock.vert");
            areaShadowMapsComp = areaShadowBuilder.build();
        } catch (ShaderLoadingException e) { std::cerr << e.what() << std::endl; }
    }

    static void renderPointLightShadowMaps(const MeshTree* meshNode, const RenderConfig& m_renderConfig, LightManager& m_lightManager) {
        // Child is a leaf, end of recursion
        if (meshNode == nullptr) return;

        const glm::mat4 pointLightShadowMapsProjection = m_renderConfig.pointShadowMapsProjectionMatrix();
        for (size_t lightIdx = 0UL; lightIdx < m_lightManager.numPointLights(); lightIdx++) {
                PointLight& light = m_lightManager.pointLightAt(lightIdx);;
                
                // Render a mesh if this node actually contains one
                if (meshNode->mesh != nullptr) {
                    const GPUMesh& mesh                         = *(meshNode->mesh);
                    const glm::mat4& modelMatrix                = meshNode->modelMatrix();
                    const std::array<glm::mat4, 6U> lightMvps   = light.genMvpMatrices(modelMatrix, pointLightShadowMapsProjection);

                    // Render each cubemap face
                    for (size_t face = 0UL; face < 6UL; face++) {
                        // Bind shadow shader and shadowmap framebuffer
                        pointShadowMapsComp.bind();
                        glBindFramebuffer(GL_FRAMEBUFFER, light.framebuffers[face]);
                        glEnable(GL_DEPTH_TEST);

                        // Bind uniforms
                        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(lightMvps[face]));
                        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(modelMatrix));
                        glUniform3fv(2, 1, glm::value_ptr(light.position));
                        glUniform1f(3, m_renderConfig.shadowFarPlane);

                        // Bind model's VAO and draw its elements
                        mesh.draw();
                    }
                }
            }

            // Recursively render each of the node's children
            for (size_t childIdx = 0; childIdx < meshNode->children.size(); childIdx++) {
                std::weak_ptr<MeshTree> child = meshNode->children.at(childIdx);
                if (child.expired()) { continue; }

                std::shared_ptr<MeshTree> childLock = child.lock();
                renderPointLightShadowMaps(childLock.get(), m_renderConfig, m_lightManager);
            }
    }

    static void renderAreaLightShadowMaps(const MeshTree* meshNode, const RenderConfig& m_renderConfig, LightManager& m_lightManager) {
        // Child is a leaf, end of recursion
        if (meshNode == nullptr) return;

        const glm::mat4 areaLightShadowMapsProjection = m_renderConfig.areaShadowMapsProjectionMatrix();
        for (size_t lightIdx = 0UL; lightIdx < m_lightManager.numAreaLights(); lightIdx++) {
            AreaLight& light = m_lightManager.areaLightAt(lightIdx);
            
            // Compute light's view matrix
            const glm::mat4 lightView = light.viewMatrix();

            // Bind shadow shader and shadowmap framebuffer
            areaShadowMapsComp.bind();
            glBindFramebuffer(GL_FRAMEBUFFER, light.framebuffer);

            // Enable depth testing to override values when necessary
            glEnable(GL_DEPTH_TEST);

            // Render a mesh if this node actually contains one
            if (meshNode->mesh != nullptr) {
                const GPUMesh& mesh             = *(meshNode->mesh);
                const glm::mat4& modelMatrix    = meshNode->modelMatrix();
                    
                // Bind light camera mvp matrix
                const glm::mat4 lightMvp = areaLightShadowMapsProjection * lightView *  modelMatrix;
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(lightMvp));

                // Bind model's VAO and draw its elements
                mesh.draw();
            }
        }

        // Recursively render each of the node's children
        for (size_t childIdx = 0; childIdx < meshNode->children.size(); childIdx++) {
            std::weak_ptr<MeshTree> child = meshNode->children.at(childIdx);
            if (child.expired()) { continue; }
            std::shared_ptr<MeshTree> childLock = child.lock();
            renderAreaLightShadowMaps(childLock.get(), m_renderConfig, m_lightManager);
        }
    }

    static void renderShadowMaps(const MeshTree* meshes, const RenderConfig& m_renderConfig, LightManager& m_lightManager) {
        // Set viewport size to fit shadow map resolution and wipe previous shadow maps
        glViewport(0, 0, utils::SHADOWTEX_WIDTH, utils::SHADOWTEX_HEIGHT);
        m_lightManager.wipeFramebuffers();

        renderPointLightShadowMaps(meshes, m_renderConfig, m_lightManager);
        renderAreaLightShadowMaps(meshes, m_renderConfig, m_lightManager);
    }
}

#endif 
