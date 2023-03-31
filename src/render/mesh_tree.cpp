#include "mesh_tree.h"
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

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

MeshTree::MeshTree(Mesh* msh, glm::vec3 off, glm::vec4 rots, glm::vec4 rotp, glm::vec3 scl) {
    this->mesh = new GPUMesh(*msh);
    this->transform = {off, rots, rotp, scl};
    this->hitBox = makeHitBox(*msh, true);
}

MeshTree::MeshTree(){
    this->transform = {glm::vec3(0.f),
                       glm::vec4(0.f,1.f,0.f,0.f),
                       glm::vec4(0.f,1.f,0.f,0.f),
                       glm::vec3(1.f)};
    this->mesh = nullptr;
}

MeshTree::MeshTree(Mesh* msh){
    this->transform = {glm::vec3(0.f),
                       glm::vec4(0.f,1.f,0.f,0.f),
                       glm::vec4(0.f,1.f,0.f,0.f),
                       glm::vec3(1.f)};
    this->mesh = new GPUMesh(*msh);
    this->hitBox = makeHitBox(*msh, true);
}

void MeshTree::addChild(MeshTree* child){
    child->parent = this;
    this->children.push_back(child);
}

glm::mat4 MeshTree::modelMatrix() {
    glm::mat4 currTransform;

    if (parent == nullptr)
        currTransform = glm::identity<glm::mat4>();
    else
        currTransform = parent->modelMatrix();

    // const MeshTransform& meshTransform = transformParams[idx];
    // const MeshTransform& meshTransform = {mt->scale, mt->selfRotate, mt->rotateParent, mt->offset};
    // Translate

    // Rotate
    glm::vec3 axis = glm::normalize(glm::vec3(transform.rotateParent.x, transform.rotateParent.y, transform.rotateParent.z));
    glm::mat4 finalTransform = glm::rotate(currTransform, glm::radians(transform.rotateParent.w), axis);
    // finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.rotateParent.y), glm::vec3(0.0f, 1.0f, 0.0f));
    // finalTransform = glm::rotate(finalTransform, glm::radians(transform.rotateParent.z), glm::vec3(0.0f, 0.0f, 1.0f));


    finalTransform = glm::translate(finalTransform, transform.translate);
    axis = glm::normalize(glm::vec3(transform.selfRotate.x, transform.selfRotate.y, transform.selfRotate.z));
    // Rotate
    finalTransform = glm::rotate(finalTransform, glm::radians(transform.selfRotate.w), axis);
    // finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
    // finalTransform = glm::rotate(finalTransform, glm::radians(meshTransform.selfRotate.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Scale
    return glm::scale(finalTransform, transform.scale);
}

HitBox MeshTree::getTransformedHitBox() {
    glm::mat4 modelMatrix = this->modelMatrix();

    HitBox transformed = this->hitBox;
    for (glm::vec3& point : transformed.points) {
        point = modelMatrix * glm::vec4(point, 1);
    }
    return transformed;
}

glm::vec3 MeshTree::getTransformedHitBoxMiddle() {
    return modelMatrix() * glm::vec4(this->hitBox.getMiddle(), 1);
}

bool MeshTree::collide(MeshTree *other) {
    return !this->hitBox.allowCollision && !other->hitBox.allowCollision
            && this->getTransformedHitBox().collides(other->getTransformedHitBox());
}

static MeshTree* collidesWith(MeshTree* root, MeshTree* toCheck) {
    if (root->collide(toCheck))
        return root;

    for (MeshTree* child : root->children) {
        if (collidesWith(child, toCheck) != nullptr)
            return child;
    }

    return nullptr;
}

// Returns true if translation succeeded
bool MeshTree::tryTranslation(glm::vec3 translation) {
    this->transform.translate += translation;

    MeshTree *root = this;
    while (root->parent != nullptr)
        root = root->parent;

    MeshTree* other = collidesWith(root, this);

    if (other == nullptr)
        return true;

    float newDist = glm::distance(this->getTransformedHitBoxMiddle(), other->getTransformedHitBoxMiddle());

    this->transform.translate -= translation;

    float oldDist = glm::distance(this->getTransformedHitBoxMiddle(), other->getTransformedHitBoxMiddle());

    if (newDist > oldDist) {
        this->transform.translate += translation;
        return true;
    }

    return false;

    // m_transformParams[idx].translate += translation;
    //
    // for (auto& [key, other] : m_hitBoxes) {
    //     if (key == idx)
    //         continue;
    //
    //     bool collidesWith = collide(idx, key);
    //
    //     if (collidesWith) {
    //         if (m_hitBoxes[idx].allowCollision && other.allowCollision)
    //             continue;
    //
    //         float newDist = glm::distance(getTransformedHitBoxMiddle(idx), getTransformedHitBoxMiddle(key));
    //
    //         m_transformParams[idx].translate -= translation;
    //         float oldDist = glm::distance(getTransformedHitBoxMiddle(idx), getTransformedHitBoxMiddle(key));
    //
    //         if (newDist < oldDist)
    //             return false;
    //         else
    //             m_transformParams[idx].translate += translation;
    //     }
    // }
    //
    // return true;
}