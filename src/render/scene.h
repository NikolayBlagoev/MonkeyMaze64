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
#include <unordered_map>
#include <map>

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
    int addMesh(std::filesystem::path filePath);
    void removeMesh(int idx);

    auto meshIterators() {
        return std::pair{meshes.begin(), meshes.end()};
    }

    glm::mat4 modelMatrix(int idx);

    std::unordered_map<int, ObjectTransform> transformParams;
    std::unordered_map<int, GPUMesh> meshes;

    HitBox getHitBox(int idx);
    glm::vec3 getHitBoxMiddle(int idx);

    bool tryUpdateScale(int idx, glm::vec3 scale);
    bool tryUpdateRotation(int idx, glm::vec3 rotation);
    bool tryUpdateTranslation(int idx, glm::vec3 translation);

private:
    std::unordered_map<int, HitBox> hitBoxes;
};

#endif
