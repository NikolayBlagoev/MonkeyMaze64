#include "mesh_tree.h"


MeshTree::MeshTree(GPUMesh* msh, glm::vec3 off, glm::vec3 rot, glm::vec3 scl) {
    this->mesh = msh;
    this->offset = off;
    this->rotate = rot;
    this->scale = scl;

}

MeshTree::MeshTree(){
    this->offset = glm::vec3(0.f);
    this->rotate = glm::vec3(0.f);
    this->scale = glm::vec3(1.f);
    this->mesh = nullptr;
}

int MeshTree::addChild(MeshTree* child){
    this->children.push_back(child);
    return this->children.size()-1;
}