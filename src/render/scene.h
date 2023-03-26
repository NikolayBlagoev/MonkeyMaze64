#ifndef _SCENE_H_
#define _SCENE_H_

#include "mesh.h"
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <filesystem>
#include <vector>

struct ObjectTransform {
    glm::vec3 scale;
    glm::vec3 rotate; // Angles in degrees
    glm::vec3 translate;
};

class Scene {
public:
    size_t addMesh(std::filesystem::path filePath);
    void removeMesh(size_t idx);

    size_t numMeshes() { return meshes.size(); }
    const GPUMesh& meshAt(size_t idx) { return meshes[idx]; }

    glm::mat4 modelMatrix(size_t idx);

    std::vector<ObjectTransform> transformParams;
    bool tryUpdateScale(size_t idx, glm::vec3 scale);
    bool tryUpdateRotation(size_t idx, glm::vec3 rotation);
    bool tryUpdateTranslation(size_t idx, glm::vec3 translation);

private:
    std::vector<GPUMesh> meshes;
};

#endif
