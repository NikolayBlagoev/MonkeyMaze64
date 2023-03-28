#include "mesh_tree.h"


MeshTree::MeshTree(GPUMesh* msh, ObjectTransform objectTransform) {
    this->mesh = msh;
    this->objectTransform = objectTransform;
}

MeshTree::MeshTree(GPUMesh* msh, glm::vec3 off, glm::vec3 rots, glm::vec3 rotp, glm::vec3 scl) {
    this->mesh = msh;

    this->objectTransform = {off, rots, rotp, scl};
}

MeshTree::MeshTree() {
    this->objectTransform = {
            glm::vec3(0.f),
            glm::vec3(0.f),
            glm::vec3(0.f),
            glm::vec3(1.f)
    };
    this->mesh = nullptr;
}

int MeshTree::addChild(MeshTree* child){
    this->children.push_back(child);
    return this->children.size()-1;
}