#include "scene.h"
#include "mesh_tree.h"
#include "mesh.h"
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

void Scene::addMesh(std::filesystem::path filePath) { 
    Mesh cpuMesh = mergeMeshes(loadMesh(filePath));
    // meshes.push_back(GPUMesh(filePath));
    // transformParams.push_back({glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f)});
    if(root == nullptr){
        root = new MeshTree();
    }
    root->addChild(new MeshTree(&cpuMesh));
}

void Scene::addMesh(MeshTree* nd){
    if(root == nullptr){
        root = new MeshTree();
    }
    root->addChild(nd);
}

//TODO: remove meshes in a sensible manner
void Scene::removeMesh(size_t idx) {
    // meshes.erase(meshes.begin() + idx);
    transformParams.erase(transformParams.begin() + idx);
}

glm::mat4 Scene::modelMatrix(size_t idx) {
    // const MeshTransform& meshTransform = transformParams[idx];
    const MeshTransform& meshTransform = root->children[idx]->transform;
    // Translate
    glm::mat4 finalTransform = glm::mat4(1.f);
    // Rotate
    glm::vec3 axis = glm::normalize(glm::vec3(meshTransform.rotateParent.x, meshTransform.rotateParent.y, meshTransform.rotateParent.z));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotateParent.w), axis);
    // finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotateParent.y), glm::vec3(0.0f, 1.0f, 0.0f));
    // finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotateParent.z), glm::vec3(0.0f, 0.0f, 1.0f));


    finalTransform = glm::translate(finalTransform, meshTransform.translate);
    axis = glm::normalize(glm::vec3(meshTransform.selfRotate.x, meshTransform.selfRotate.y, meshTransform.selfRotate.z));
    // Rotate
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.w), axis);
    // finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
    // finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Scale
    return glm::scale(finalTransform, meshTransform.scale);
}


