#include "scene.h"
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

size_t Scene::addMesh(std::filesystem::path filePath) {
    meshes.push_back(GPUMesh(filePath));
    transformParams.push_back({glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f)});

    return meshes.size() - 1;
}

void Scene::removeMesh(size_t idx) {
    meshes.erase(meshes.begin() + idx);
    transformParams.erase(transformParams.begin() + idx);
}

glm::mat4 Scene::modelMatrix(size_t idx) {
    const ObjectTransform& transform = transformParams[idx];

    // Translate
    glm::mat4 finalTransform = glm::translate(transform.translate);

    // Rotate
    finalTransform = glm::rotate(finalTransform, glm::radians(transform.rotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(transform.rotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(transform.rotate.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Scale
    return glm::scale(finalTransform, transform.scale);
}

bool Scene::tryUpdateScale(size_t idx, glm::vec3 scale) {
    transformParams[idx].scale += scale;
    return true;
}

bool Scene::tryUpdateRotation(size_t idx, glm::vec3 rotation) {
    transformParams[idx].rotate += rotation;
    return true;
}

bool Scene::tryUpdateTranslation(size_t idx, glm::vec3 translation) {
    glm::vec3 newPos = transformParams[idx].translate + translation;

    // TODO

    transformParams[idx].translate = newPos;
    return true;
}


