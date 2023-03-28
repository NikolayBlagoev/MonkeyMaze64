#include "scene.h"
#include "framework/mesh.h"

DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

static HitBox makeHitBox(std::filesystem::path filePath) {
    const Mesh cpuMesh = mergeMeshes(loadMesh(filePath));

    std::array<std::array<float, 3>, 2> minMax{};

    for (Vertex vertex : cpuMesh.vertices) {
        if (vertex.position.x < minMax[0][0])
            minMax[0][0] = vertex.position.x;
        if (vertex.position.x > minMax[1][0])
            minMax[1][0] = vertex.position.x;

        if (vertex.position.y < minMax[0][1])
            minMax[0][1] = vertex.position.y;
        if (vertex.position.y > minMax[1][1])
            minMax[1][1] = vertex.position.y;

        if (vertex.position.z < minMax[0][2])
            minMax[0][2] = vertex.position.z;
        if (vertex.position.z > minMax[1][2])
            minMax[1][2] = vertex.position.z;
    }

    std::array<glm::vec3, 8> points{};

    for (int z = 0; z < 2; ++z) {
        for (int y = 0; y < 2; ++y) {
            for (int x = 0; x < 2; ++x) {
                points[x + y * 2 + z * 4] = glm::vec3(minMax[x][0], minMax[y][1], minMax[z][2]);
            }
        }
    }

    return {points};
}

static int latestKey = 0;

int Scene::addMesh(std::filesystem::path filePath) {
    int key = latestKey++;

    meshes.insert({key, GPUMesh(filePath)});
    transformParams.insert({key, {glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f)}});
    hitBoxes.insert({key, makeHitBox(filePath)});

    return key;
}

void Scene::removeMesh(int idx) {
    meshes.erase(idx);
    transformParams.erase(idx);
    hitBoxes.erase(idx);
}

glm::mat4 Scene::modelMatrix(int idx) {
    const ObjectTransform& transform = transformParams[idx];

    // Translate
    glm::mat4 finalTransform = glm::translate(transform.translate);

    // Rotate
    finalTransform = glm::rotate(finalTransform, glm::radians(transform.rotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(transform.rotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(transform.rotate.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Scale
    return glm::scale(finalTransform, transform.scale);
}

HitBox Scene::getHitBox(int idx) {
    HitBox hitBox = hitBoxes[idx];
    for (glm::vec3& point : hitBox.points) {
        point = modelMatrix(idx) * glm::vec4(point, 1);
    }
    return hitBox;
}

glm::vec3 Scene::getHitBoxMiddle(int idx) {
    return modelMatrix(idx) * glm::vec4(hitBoxes[idx].getMiddle(), 1);
}

bool Scene::tryUpdateScale(int idx, glm::vec3 scale) {
//    transformParams[idx].scale += scale;
    return true;
}

bool Scene::tryUpdateRotation(int idx, glm::vec3 rotation) {
//    transformParams[idx].rotate += rotation;
    return true;
}

bool Scene::tryUpdateTranslation(int idx, glm::vec3 translation) {
    transformParams[idx].translate += translation;
    HitBox newHitBox = getHitBox(idx);
auto bla = transformParams.begin();
    for (auto& [key, curHitBox] : hitBoxes) {
        if (key == idx)
            continue;

        for (glm::vec3 newPoint : newHitBox.points) {

            glm::vec3 smallerOrEqual(0);
            glm::vec3 bigger(0);

            for (glm::vec3 curPoint : curHitBox.points) {
                if (newPoint.x <= curPoint.x)
                    smallerOrEqual.x++;
                else
                    bigger.x++;

                if (newPoint.y <= curPoint.y)
                    smallerOrEqual.y++;
                else
                    bigger.y++;

                if (newPoint.z <= curPoint.z)
                    smallerOrEqual.z++;
                else
                    bigger.z++;
            }

            bool collision =    (smallerOrEqual.x != 0 && bigger.x != 0)
                             && (smallerOrEqual.y != 0 && bigger.y != 0)
                             && (smallerOrEqual.z != 0 && bigger.z != 0);

            if (collision) {
                float newDist = glm::distance(getHitBoxMiddle(idx), getHitBoxMiddle(key));

                transformParams[idx].translate -= translation;
                float oldDist = glm::distance(getHitBoxMiddle(idx), getHitBoxMiddle(key));

                if (newDist < oldDist)
                    return false;
                else
                    transformParams[idx].translate += translation;
            }
        }
    }

    return true;
}


