#ifndef _MESH_TREE_H_
#define _MESH_TREE_H_

#include "mesh.h"
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <filesystem>
#include <vector>



class MeshTree {
public:
    MeshTree(GPUMesh* msh, glm::vec3 off, glm::vec3 rots, glm::vec3 rotp, glm::vec3 scl);
    int addChild(MeshTree* child);
    MeshTree();
    // void addMesh(std::filesystem::path filePath);
    // void removeMesh(size_t idx);

    // size_t numMeshes() { return meshes.size(); }
    // const GPUMesh& meshAt(size_t idx) { return meshes[idx]; }

    // glm::mat4 modelMatrix(size_t idx);

    // std::vector<MeshTransform> transformParams;

public:
    GPUMesh* mesh;
    glm::vec3 offset;
    glm::vec3 selfRotate;
    glm::vec3 rotateParent;
    glm::vec3 scale;
    std::vector<MeshTree*> children;
};

#endif
