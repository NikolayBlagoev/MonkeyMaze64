#include "scene.h"
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

void Scene::addMesh(std::filesystem::path filePath) { 
    meshes.push_back(GPUMesh(filePath));
    transformParams.push_back({glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f)});
}

void Scene::removeMesh(size_t idx) {
    meshes.erase(meshes.begin() + idx);
    transformParams.erase(transformParams.begin() + idx);
}

glm::mat4 Scene::modelMatrix(size_t idx) {
    const MeshTransform& meshTransform = transformParams[idx];

    // Translate
    glm::mat4 finalTransform = glm::translate(meshTransform.translate);

    // Rotate
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotate.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Scale
    return glm::scale(finalTransform, meshTransform.scale);
}
