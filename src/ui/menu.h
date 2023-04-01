#ifndef _MENU_H_
#define _MENU_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>

#include <render/deferred.h>
#include <render/config.h>
#include <render/lighting.h>
#include <render/particle.h>
#include <render/scene.h>

class Menu {
public:
    Menu(Scene& scene, RenderConfig& renderConfig, LightManager& lightManager,
         ParticleEmitterManager& particleEmitterManager, DeferredRenderer& deferredRenderer);

    // Main drawing functions
    void draw2D();
    void draw3D(const glm::mat4& cameraMVP);

private:
    /************ 2D UI ***********/ 
    void drawGameMechanicsTab();

    void drawCameraTab();

    // Mesh tab
    void addMesh();
    void drawMaterialControls();
    void drawMeshControls();
    void drawMeshTab();

    // Light tab
    void drawGeneralLightControls();
    void drawPointLightControls();
    void drawAreaLightControls();
    void drawLightTab();

    // Shadow tab
    void drawShadowTab();

    // Particle tab
    void drawParticleParamControls();
    void drawEmitterControls();
    void drawParticleTab();

    // Shader tab
    void drawShaderLoader();
    void drawToonShadingControls();
    void drawShadingTab();

    // Render tab
    void drawHdrControls();
    void drawBloomControls();
    void drawParallaxControls();
    void drawRenderTab();
    /******************************/ 

    // 3D debug view
    void drawPoint(float radius, const glm::vec4& screenPos, const glm::vec4& color);
    void drawPoint(float radius, const glm::vec4& screenPos, const glm::vec3& color) { drawPoint(radius, screenPos, glm::vec4(color, 1.0f)); }
    void drawLights(const glm::mat4& cameraMVP);
    void drawParticleEmitters(const glm::mat4& cameraMVP);

    RenderConfig& m_renderConfig;
    Scene& m_scene;
    LightManager& m_lightManager;
    ParticleEmitterManager& m_particleEmitterManager;
    DeferredRenderer& m_deferredRenderer;

    Shader debugShader;

    size_t selectedMesh                 { 0U };
    size_t selectedPointLight           { 0U };
    size_t selectedAreaLight            { 0U };
    size_t selectedParticleEmitter      { 0U };
    LightingModel selectedLightingModel { LightingModel::PBR };
};

#endif
