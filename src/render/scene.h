#ifndef _SCENE_H_
#define _SCENE_H_
#include "mesh_tree.h"
#include "mesh.h"
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <filesystem>
#include <vector>

struct MeshTransform {
    glm::vec3 scale;
    glm::vec3 rotate; // Angles in degrees
    glm::vec3 translate;
};

class Scene {
public:
    void addMesh(std::filesystem::path filePath);
    void removeMesh(size_t idx);
    void addMesh(MeshTree* nd);
    size_t numMeshes() { return meshes.size(); }
    const GPUMesh& meshAt(size_t idx) { return *(root->children[idx]->mesh); }

    glm::mat4 modelMatrix(size_t idx);
    MeshTree* root;
    std::vector<MeshTransform> transformParams;

private:
    std::vector<GPUMesh> meshes;
    
    
};

#endif
