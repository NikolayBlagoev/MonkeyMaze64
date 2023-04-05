#include "scene.h"
#include "mesh_tree.h"
#include "mesh.h"
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

void Scene::addMesh(std::shared_ptr<MeshTree> nd){
    if (root == nullptr) {
        root = new MeshTree("root", std::nullopt);
        MemoryManager::addEl(root);
        root->is_root = true;
    }
    root->addChild(nd);
}
