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
#include <optional>
#include <array>

struct ObjectTransform {
    glm::vec3 scale;
    glm::vec3 rotate; // Angles in degrees
    glm::vec3 translate;
};

struct HitBox {
    std::array<glm::vec3, 8> points;

    glm::vec3 getMiddle() {
        // assume corner points are first and last
        return {(points[0].x + points[7].x) / 2,
                (points[0].y + points[7].y) / 2,
                (points[0].z + points[7].z) / 2};
    }
};

class Scene {
public:
    size_t addMesh(std::filesystem::path filePath);
    void removeMesh(size_t idx);

    size_t numMeshes() { return meshes.size(); }
    const GPUMesh& meshAt(size_t idx) { return meshes[idx]; }

    glm::mat4 modelMatrix(size_t idx);

    std::vector<ObjectTransform> transformParams;

    HitBox getHitBox(size_t idx);
    glm::vec3 getHitBoxMiddle(size_t idx);

    bool tryUpdateScale(size_t idx, glm::vec3 scale);
    bool tryUpdateRotation(size_t idx, glm::vec3 rotation);
    bool tryUpdateTranslation(size_t idx, glm::vec3 translation);

private:
    std::vector<GPUMesh> meshes;
    std::vector<HitBox> hitBoxes;
};

#endif
