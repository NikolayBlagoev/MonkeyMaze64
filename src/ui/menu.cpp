#include "menu.h"
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <nativefiledialog/nfd.h>
DISABLE_WARNINGS_POP()
#include <filesystem>
#include <iostream>

Menu::Menu(Scene& scene) : m_scene(scene) {}

void Menu::draw() {
    ImGui::Begin("Debug Controls");
    
    drawObjectControls();

    ImGui::End();
}

void Menu::addObject() {
    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog(NULL, RESOURCES_DIR, &outPath );
        
    if (result == NFD_OKAY) {
        std::filesystem::path objPath(outPath);
        m_scene.addMesh(objPath);
        free(outPath);
    } else if (result == NFD_CANCEL) { std::cout << "Model loading cancelled" << std::endl; }
      else { std::cerr << "Model loading error" << std::endl; }
}

void Menu::drawObjectControls() {
    // Add / remove controls
    if (ImGui::Button("Add")) { addObject(); }
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

    // Selected light controls
    if (m_scene.numMeshes() > 0) {
        ImGui::DragFloat3("Scale", glm::value_ptr(m_scene.transformParams[selectedMesh].scale), 0.05f);
        ImGui::DragFloat3("Rotate", glm::value_ptr(m_scene.transformParams[selectedMesh].rotate), 1.0f, 0.0f, 360.0f);
        ImGui::DragFloat3("Translate", glm::value_ptr(m_scene.transformParams[selectedMesh].translate), 0.05f);
    }
}
