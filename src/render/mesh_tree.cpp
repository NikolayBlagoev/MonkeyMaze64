#include "mesh_tree.h"


MeshTree::MeshTree(GPUMesh* msh, glm::vec3 off, glm::vec4 rots, glm::vec4 rotp, glm::vec3 scl) {
    this->mesh = msh;
    this->offset = off;
    this->selfRotate = rots;
    this-> rotateParent = rotp;
    this->scale = scl;

}

MeshTree::MeshTree(){
    this->offset = glm::vec3(0.f);
    this->selfRotate = glm::vec4(0.f,1.f,0.f,0.f);
    this->rotateParent = glm::vec4(0.f,1.f,0.f,0.f);
    this->scale = glm::vec3(1.f);
    this->mesh = nullptr;
}

MeshTree::MeshTree(GPUMesh* msh){
    this->offset = glm::vec3(0.f);
    this->selfRotate = glm::vec4(0.f,1.f,0.f,0.f);
    this->rotateParent = glm::vec4(0.f,1.f,0.f,0.f);
    this->scale = glm::vec3(1.f);
    this->mesh = msh;
}

int MeshTree::addChild(MeshTree* child){
    this->children.push_back(child);
    return this->children.size()-1;
}