#include "menu.h"

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <nativefiledialog/nfd.h>
DISABLE_WARNINGS_POP()
#include <utils/constants.h>

#include <filesystem>
#include <iostream>

Menu::Menu(Scene& scene, RenderConfig& renderConfig, LightManager& lightManager) : 
    m_scene(scene),
    m_renderConfig(renderConfig),
    m_lightManager(lightManager) {
    ShaderBuilder debugShaderBuilder;
    debugShaderBuilder.addStage(GL_VERTEX_SHADER, utils::SHADERS_DIR_PATH / "debug" / "light_debug.vert");
    debugShaderBuilder.addStage(GL_FRAGMENT_SHADER, utils::SHADERS_DIR_PATH  / "debug" / "light_debug.frag");
    pointDebugShader = debugShaderBuilder.build();
}

void Menu::draw(const glm::mat4& cameraMVP) {
    draw2D();
    draw3D(cameraMVP);
}

void Menu::draw2D() {
    ImGui::Begin("Debug Controls");
    ImGui::BeginTabBar("Categories");
    
    drawCameraTab();
    drawMeshTab();
    drawLightTab();
    drawShadowTab();

    ImGui::EndTabBar();
    ImGui::End();
}

void Menu::draw3D(const glm::mat4& cameraMVP) {
    drawLights(cameraMVP);
}

void Menu::drawCameraTab() {
    if (ImGui::BeginTabItem("Camera")) {
        ImGui::DragFloat("Movement Speed", &m_renderConfig.moveSpeed, 0.001f, 0.01f, 0.09f);
        ImGui::DragFloat("Look Speed", &m_renderConfig.lookSpeed, 0.0001f, 0.0005f, 0.0050f);
        ImGui::DragFloat("FOV (Vertical)", &m_renderConfig.verticalFOV, 1.0f, 30.0f, 180.0f);
        ImGui::DragFloat("Zoomed FOV (Vertical)", &m_renderConfig.zoomedVerticalFOV, 1.0f, 20.0f, 120.0f);
        ImGui::Checkbox("Constrain Vertical Movement", &m_renderConfig.constrainVertical);
        ImGui::EndTabItem();
    }
}

void Menu::addMesh() {
    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath );
        
    if (result == NFD_OKAY) {
        std::filesystem::path objPath(outPath);
        m_scene.addMesh(objPath);
        free(outPath);
    } else if (result == NFD_CANCEL) { std::cout << "Model loading cancelled" << std::endl; }
      else { std::cerr << "Model loading error" << std::endl; }
}

void Menu::drawMeshTab() {
    if (ImGui::BeginTabItem("Meshes")) {
        // Add / remove controls
        if (ImGui::Button("Add")) { addMesh(); }
        if (ImGui::Button("Remove selected")) {
            if (selectedMesh < m_scene.numMeshes()) {
                m_scene.removeMesh(selectedMesh);
                selectedMesh = 0U;
            }
        }
        ImGui::NewLine();

        // Selection controls
        std::vector<std::string> options;
        for (size_t meshIdx = 0U; meshIdx < m_scene.numMeshes(); meshIdx++) { options.push_back("Mesh " + std::to_string(meshIdx + 1)); }
        std::vector<const char*> optionsPointers;
        std::transform(std::begin(options), std::end(options), std::back_inserter(optionsPointers),
            [](const auto& str) { return str.c_str(); });
        ImGui::Combo("Selected mesh", (int*) (&selectedMesh), optionsPointers.data(), static_cast<int>(optionsPointers.size()));

        // Selected mesh controls
        if (m_scene.numMeshes() > 0U) {
            ImGui::DragFloat3("Scale", glm::value_ptr(m_scene.transformParams[selectedMesh].scale), 0.05f);
            ImGui::DragFloat3("Rotate", glm::value_ptr(m_scene.transformParams[selectedMesh].rotate), 1.0f, 0.0f, 360.0f);
            ImGui::DragFloat3("Translate", glm::value_ptr(m_scene.transformParams[selectedMesh].translate), 0.05f);
        }

        ImGui::EndTabItem();
    }
}

void Menu::drawGeneralLightControls() {
    ImGui::Checkbox("Draw all lights", &m_renderConfig.drawLights);
}

// (hashes prevent ID conflicts https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-have-multiple-windows-with-the-same-label)
void Menu::drawPointLightControls() {
    // Add / remove / draw controls
    if (ImGui::Button("Add##point")) { m_lightManager.addPointLight(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)); }
    if (ImGui::Button("Remove selected##point")) {
        if (selectedPointLight < m_lightManager.numPointLights()) {
            m_lightManager.removePointLight(selectedPointLight);
            selectedPointLight = 0U;
        }
    }
    ImGui::Checkbox("Highlight selected##point", &m_renderConfig.drawSelectedPointLight);
    ImGui::NewLine();

    // Selection controls
    std::vector<std::string> options;
    for (size_t pointLightIdx = 0U; pointLightIdx < m_lightManager.numPointLights(); pointLightIdx++) { options.push_back("Point light " + std::to_string(pointLightIdx + 1)); }
    std::vector<const char*> optionsPointers;
    std::transform(std::begin(options), std::end(options), std::back_inserter(optionsPointers),
        [](const auto& str) { return str.c_str(); });
    ImGui::Combo("Selected point light", (int*) (&selectedPointLight), optionsPointers.data(), static_cast<int>(optionsPointers.size()));

    // Selected point light controls
    if (m_lightManager.numPointLights() > 0U) {
        PointLight& selectedLight = m_lightManager.pointLightAt(selectedPointLight);
        ImGui::ColorEdit3("Colour##point", glm::value_ptr(selectedLight.color));
        ImGui::DragFloat3("Position##point", glm::value_ptr(selectedLight.position), 0.05f);
    }
}

// (hashes prevent ID conflicts https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-have-multiple-windows-with-the-same-label)
void Menu::drawAreaLightControls() {
    // Add / remove / draw controls
    if (ImGui::Button("Add##area")) { m_lightManager.addAreaLight(glm::vec3(0.0f), glm::vec3(1.0f)); }
    if (ImGui::Button("Remove selected##area")) {
        if (selectedAreaLight < m_lightManager.numAreaLights()) {
            m_lightManager.removeAreaLight(selectedAreaLight);
            selectedAreaLight = 0U;
        }
    }
    ImGui::Checkbox("Highlight selected##area", &m_renderConfig.drawSelectedAreaLight);
    ImGui::NewLine();

    // Selection controls
    std::vector<std::string> options;
    for (size_t areaLightIdx = 0U; areaLightIdx < m_lightManager.numAreaLights(); areaLightIdx++) { options.push_back("Area light " + std::to_string(areaLightIdx + 1)); }
    std::vector<const char*> optionsPointers;
    std::transform(std::begin(options), std::end(options), std::back_inserter(optionsPointers),
        [](const auto& str) { return str.c_str(); });
    ImGui::Combo("Selected area light", (int*) (&selectedAreaLight), optionsPointers.data(), static_cast<int>(optionsPointers.size()));

    // Selected area light controls (hashes prevent ID conflicts https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-have-multiple-windows-with-the-same-label)
    if (m_lightManager.numAreaLights() > 0U) {
        AreaLight& selectedLight = m_lightManager.areaLightAt(selectedAreaLight);
        ImGui::ColorEdit3("Colour##area", glm::value_ptr(selectedLight.color));
        ImGui::DragFloat3("Position##area", glm::value_ptr(selectedLight.position), 0.05f);
        ImGui::SliderFloat("X Rotation", &selectedLight.rotX, 0.0f, 360.0f);
        ImGui::SliderFloat("Y Rotation", &selectedLight.rotY, 0.0f, 360.0f);
    }
}

void Menu::drawLightTab() {
    if (ImGui::BeginTabItem("Lights")) {
        ImGui::Text("General");
        drawGeneralLightControls();

        ImGui::NewLine();
        ImGui::Separator();

        ImGui::Text("Point lights");
        drawPointLightControls();

        ImGui::NewLine();
        ImGui::Separator();

        ImGui::Text("Area lights");
        drawAreaLightControls();

        ImGui::EndTabItem();
    }
}

void Menu::drawShadowTab() {
    if (ImGui::BeginTabItem("Shadows")) {
        ImGui::SliderFloat("Shadow Map FoV (Vertical)", &m_renderConfig.areaShadowFovY, 40.0f, 150.0f);
        ImGui::SliderFloat("Shadow Map Near Plane", &m_renderConfig.shadowNearPlane, 0.0001f, 1.0f);
        ImGui::SliderFloat("Shadow Map Far Plane", &m_renderConfig.shadowFarPlane, 5.0f, 60.0f);
        ImGui::EndTabItem();
    }
}

void Menu::drawPoint(float radius, const glm::vec4& screenPos, const glm::vec4& color) {
    glPointSize(radius);
    glUniform4fv(0, 1, glm::value_ptr(screenPos));
    glUniform4fv(1, 1, glm::value_ptr(color));
    glDrawArrays(GL_POINTS, 0, 1);
}

void Menu::drawLights(const glm::mat4& cameraMVP) {
    pointDebugShader.bind();

    // Draw all lights
    if (m_renderConfig.drawLights) {
        // Point lights
        for (size_t lightIdx = 0U; lightIdx < m_lightManager.numPointLights(); lightIdx++) {
            const PointLight& light     = m_lightManager.pointLightAt(lightIdx);
            const glm::vec4 screenPos   = cameraMVP * glm::vec4(light.position.x, light.position.y, light.position.z, 1.0f);
            drawPoint(10.0f, screenPos, light.color);
        }

        // Area lights
        for (size_t lightIdx = 0U; lightIdx < m_lightManager.numAreaLights(); lightIdx++) {
            const AreaLight& light          = m_lightManager.areaLightAt(lightIdx);
            const glm::vec4 screenPosStart  = cameraMVP * glm::vec4(light.position, 1.0f);
            const glm::vec4 screenPosEnd    = cameraMVP * glm::vec4(light.position + light.forwardDirection(), 1.0f);
            drawPoint(25.0f, screenPosStart, light.color);
            drawPoint(15.0f, screenPosEnd, light.color);
        }
    }

    // Draw selected point light if it exists
    if (m_renderConfig.drawSelectedPointLight && m_lightManager.numPointLights() > 0U) {
        const PointLight& light     = m_lightManager.pointLightAt(selectedPointLight);
        const glm::vec4 screenPos   = cameraMVP * glm::vec4(light.position.x, light.position.y, light.position.z, 1.0f);
        drawPoint(40.0f, screenPos, glm::vec3(1.0f, 1.0f, 1.0f));
    }

    // Draw selected area light if it exists
    if (m_renderConfig.drawSelectedAreaLight && m_lightManager.numAreaLights() > 0U) {
        const AreaLight& light      = m_lightManager.areaLightAt(selectedAreaLight);
        const glm::vec4 screenPos   = cameraMVP * glm::vec4(light.position.x, light.position.y, light.position.z, 1.0f);
        drawPoint(40.0f, screenPos, glm::vec3(1.0f, 1.0f, 1.0f));
    }
}
