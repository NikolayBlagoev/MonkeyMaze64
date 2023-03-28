#include "mesh_tree.h"


MeshTree::MeshTree(GPUMesh* msh, glm::vec3 off, glm::vec3 rots, glm::vec3 rotp, glm::vec3 scl) {
    this->mesh = msh;
    this->offset = off;
    this->selfRotate = rots;
    this-> rotateParent = rotp;
    this->scale = scl;

}

MeshTree::MeshTree(){
    this->offset = glm::vec3(0.f);
    this->selfRotate = glm::vec3(0.f);
    this->rotateParent = glm::vec3(0.f);
    this->scale = glm::vec3(1.f);
    this->mesh = nullptr;
}

int MeshTree::addChild(MeshTree* child){
    this->children.push_back(child);
    return this->children.size()-1;
}