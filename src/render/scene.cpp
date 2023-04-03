#include "scene.h"
#include "mesh_tree.h"
#include "mesh.h"
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

void Scene::addMesh(std::filesystem::path filePath) { 
    Mesh cpuMesh = mergeMeshes(loadMesh(filePath));

    if(root == nullptr){
        root = new MeshTree("root");
        MemoryManager::addEl(root);
        root->is_root = true;
    }
    MeshTree* newMesh = new MeshTree(filePath.filename(), &cpuMesh);
    root->addChild(newMesh->shared_from_this());
}

void Scene::addMesh(std::shared_ptr<MeshTree> nd){
    if(root == nullptr){
        root = new MeshTree("root");
        MemoryManager::addEl(root);
        root->is_root = true;
    }
    root->addChild(nd);
}

//TODO: remove meshes in a sensible manner
void Scene::removeMesh(size_t idx) {
    // meshes.erase(meshes.begin() + idx);
    transformParams.erase(transformParams.begin() + idx);
}
