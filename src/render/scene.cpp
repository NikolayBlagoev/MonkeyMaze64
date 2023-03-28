#include "scene.h"

#include "framework/mesh.h"
#include "fmt/format.h"
#include "mesh_tree.h"
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

// collision branch start
static HitBox makeHitBox(Mesh& cpuMesh, bool allowCollision) {
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

    return {allowCollision, points};
}

static int latestKey = 0;

int Scene::addMesh(std::filesystem::path filePath, bool allowCollision) {
    if (!std::filesystem::exists(filePath))
        throw MeshLoadingException(fmt::format("File {} does not exist", filePath.string().c_str()));
    
    
    // Defined in <framework/mesh.h>
    Mesh cpuMesh = mergeMeshes(loadMesh(filePath));
    
    int key = latestKey++;
    GPUMesh* gp = new GPUMesh(cpuMesh);
    // m_meshes.insert({key, GPUMesh(cpuMesh)});
    // m_transformParams.insert({key, {glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f)}});
    m_hitBoxes.insert({key, makeHitBox(cpuMesh, allowCollision)});
    if(root == nullptr){
        root = new MeshTree();
    }
    root->addChild(new MeshTree(gp, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f)));
    return key;
}



//glm::mat4 Scene::modelMatrix(int idx) {
//    const ObjectTransform& transform = m_transformParams[idx];
//
//    // Translate
//    glm::mat4 finalTransform = glm::translate(transform.translate);
//
//    // Rotate
//    finalTransform = glm::rotate(finalTransform, glm::radians(transform.rotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
//    finalTransform = glm::rotate(finalTransform, glm::radians(transform.rotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
//    finalTransform = glm::rotate(finalTransform, glm::radians(transform.rotate.z), glm::vec3(0.0f, 0.0f, 1.0f));

// collision branch end



void Scene::addMesh(MeshTree* nd){
    if(root == nullptr){
        root = new MeshTree();
    }
    root->addChild(nd);
}

//TODO: remove meshes in a sensible manner
void Scene::removeMesh(size_t idx) {
    // meshes.erase(meshes.begin() + idx);
    // transformParams.erase(transformParams.begin() + idx);
    m_hitBoxes.erase(idx);
}

glm::mat4 Scene::modelMatrix(size_t idx) {
    // const MeshTransform& meshTransform = transformParams[idx];
    const ObjectTransform& meshTransform = {root->children[idx]->scale, root->children[idx]->selfRotate, root->children[idx]->rotateParent, root->children[idx]->offset};
    // Translate
    glm::mat4 finalTransform = glm::mat4(1.f);
    // Rotate
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotateParent.x), glm::vec3(1.0f, 0.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotateParent.y), glm::vec3(0.0f, 1.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotateParent.z), glm::vec3(0.0f, 0.0f, 1.0f));


    finalTransform = glm::translate(finalTransform, meshTransform.translate);

    // Rotate
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
    finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Scale
    return glm::scale(finalTransform, meshTransform.scale);
}

HitBox Scene::getTransformedHitBox(int idx) {
    HitBox hitBox = m_hitBoxes[idx];
    for (glm::vec3& point : hitBox.points) {
        point = modelMatrix(idx) * glm::vec4(point, 1);
    }
    return hitBox;
}

glm::vec3 Scene::getTransformedHitBoxMiddle(int idx) {
    return modelMatrix(idx) * glm::vec4(m_hitBoxes[idx].getMiddle(), 1);
}

bool Scene::collide(int a, int b) {
    HitBox hitBoxA = getTransformedHitBox(a);
    HitBox hitBoxB = getTransformedHitBox(b);

    for (glm::vec3 newPoint : hitBoxA.points) {

        glm::vec3 smallerOrEqual(0);
        glm::vec3 bigger(0);

        for (glm::vec3 curPoint: hitBoxB.points) {
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

        bool collision = (smallerOrEqual.x != 0 && bigger.x != 0)
                         && (smallerOrEqual.y != 0 && bigger.y != 0)
                         && (smallerOrEqual.z != 0 && bigger.z != 0);

        if (collision)
            return true;
    }

    return false;
}

bool Scene::checkScaleValid(int idx, glm::vec3 scale) {
//    m_transformParams[idx].scale += scale;
    return true;
}

bool Scene::checkRotationValid(int idx, glm::vec3 rotation) {
//    m_transformParams[idx].rotate += rotation;
    return true;
}

bool Scene::checkTranslationValid(int idx, glm::vec3 translation) {
    m_transformParams[idx].translate += translation;

    for (auto& [key, other] : m_hitBoxes) {
        if (key == idx)
            continue;

        bool collision = collide(idx, key);

        if (collision) {
            if (m_hitBoxes[idx].allowCollision && other.allowCollision)
                continue;

            float newDist = glm::distance(getTransformedHitBoxMiddle(idx), getTransformedHitBoxMiddle(key));

            m_transformParams[idx].translate -= translation;
            float oldDist = glm::distance(getTransformedHitBoxMiddle(idx), getTransformedHitBoxMiddle(key));

            if (newDist < oldDist)
                return false;
            else
                m_transformParams[idx].translate += translation;
        }
    }

    return true;
}


