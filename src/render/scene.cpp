#include "scene.h"
#include "mesh_tree.h"
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

void Scene::addMesh(std::filesystem::path filePath) { 
    GPUMesh* gp = new GPUMesh(filePath);
    // meshes.push_back(GPUMesh(filePath));
    // transformParams.push_back({glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f)});
    if(root == nullptr){
        root = new MeshTree();
    }
    root->addChild(new MeshTree(gp));
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
    const MeshTransform& meshTransform = {root->children[idx]->scale, root->children[idx]->selfRotate, root->children[idx]->rotateParent, root->children[idx]->offset};
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

glm::mat4 Scene::modelMatrix(MeshTree* mt, const glm::mat4& currTransform) {
    // const MeshTransform& meshTransform = transformParams[idx];
    const MeshTransform& meshTransform = {mt->scale, mt->selfRotate, mt->rotateParent, mt->offset};
    // Translate
     
    // Rotate
    glm::vec3 axis = glm::normalize(glm::vec3(meshTransform.rotateParent.x, meshTransform.rotateParent.y, meshTransform.rotateParent.z));
    glm::mat4 finalTransform = glm::rotate(currTransform, glm::radians(meshTransform.rotateParent.w), axis);
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
