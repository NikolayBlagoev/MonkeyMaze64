#include "mesh_tree.h"

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

#include <iostream>

std::unordered_map<MeshTree*, std::shared_ptr<MeshTree>> MemoryManager::objs;

MeshTree::MeshTree(std::string tag, Mesh* msh, glm::vec3 off, glm::vec4 rots, glm::vec4 rotp, glm::vec3 scl, bool allowCollision) {
    this->transform = {off, rots, rotp, scl};
    this->tag = tag;

    if (msh != nullptr) {
        this->mesh      = new GPUMesh(*msh);
        this->hitBox    = HitBox::makeHitBox(*msh, allowCollision);
    }
}

MeshTree::~MeshTree() { std::cout<< "Destructor called for MeshTree with tag: " << tag << std::endl; }

void MeshTree::addChild(std::shared_ptr<MeshTree> child){
    child.get()->parent = std::weak_ptr<MeshTree>(shared_from_this());
    this->children.push_back(child);
}

glm::mat4 MeshTree::modelMatrix() const {
    glm::mat4 currTransform;

    // Apply model matrix of parent
    if (is_root) { currTransform = glm::identity<glm::mat4>(); }
    else {
        if (parent.expired()) {
            std::cerr << "WARNING! EXPIRED PARENT!!" << std::endl;
            return glm::identity<glm::mat4>();
        }
        std::shared_ptr parentPtr = parent.lock();
        currTransform = parentPtr.get()->modelMatrix();
    }

    // Rotate relative to parent
    currTransform = glm::rotate(currTransform, glm::radians(transform.rotateParent.w), glm::vec3(transform.rotateParent.x, transform.rotateParent.y, transform.rotateParent.z));

    // Translate (also translate external object(s) if any)
    currTransform = glm::translate(currTransform, transform.translate);
    if (al != nullptr) {
        al->position        = currTransform * glm::vec4(0.f, 0.f, 0.f, 1.f);
        al->externalForward = glm::normalize(currTransform * glm::vec4(-1.f, 0.f, 0.f, 1.f));
    }

    // Rotate
    currTransform = glm::rotate(currTransform, glm::radians(transform.selfRotate.w), glm::vec3(transform.selfRotate.x, transform.selfRotate.y, transform.selfRotate.z));

    // Scale
    return glm::scale(currTransform, transform.scale);
}

HitBox MeshTree::getTransformedHitBox() {
    glm::mat4 modelMatrix   = this->modelMatrix();
    HitBox transformed      = this->hitBox;
    for (glm::vec3& point : transformed.points) { point = modelMatrix * glm::vec4(point, 1.0f); }
    return transformed;
}

glm::vec3 MeshTree::getTransformedHitBoxMiddle() { return modelMatrix() * glm::vec4(this->hitBox.getMiddle(), 1); }

bool MeshTree::collide(MeshTree *other) {
    return (!this->hitBox.allowCollision || !other->hitBox.allowCollision) &&
             this->getTransformedHitBox().collides(other->getTransformedHitBox());
}

static MeshTree* collidesWith(MeshTree* root, MeshTree* toCheck) {
    if (root->collide(toCheck)) { return root; }

    for (size_t childIdx = 0; childIdx < root->children.size(); childIdx++) {
        std::weak_ptr<MeshTree> child = root->children.at(childIdx);
        if (child.expired()) { continue; }
        MeshTree* childLock = child.lock().get();
        if (collidesWith(childLock, toCheck) != nullptr) { return childLock; }
    }

    return nullptr;
}

// Returns true if translation succeeded
bool MeshTree::tryTranslation(glm::vec3 translation, MeshTree* root) {
    this->transform.translate += translation;

    MeshTree* other = collidesWith(root, this);
    if (other == nullptr) { return true; }

    float newDist = glm::distance(this->getTransformedHitBoxMiddle(), other->getTransformedHitBoxMiddle());
    this->transform.translate -= translation;
    float oldDist = glm::distance(this->getTransformedHitBoxMiddle(), other->getTransformedHitBoxMiddle());

    if (newDist > oldDist) {
        this->transform.translate += translation;
        return true;
    }

    return false;
}

void MeshTree::clean(LightManager& lmngr){
    // Destroy all children
    for (size_t childIdx = 0; childIdx < children.size(); childIdx++) {
        if (!children[childIdx].expired()) {
            MeshTree* child = children[childIdx].lock().get();
            MemoryManager::removeEl(child);
        }
    }

    // Destroy all external objects (if any)
    if (al != nullptr){ lmngr.removeByReference(al); }
}
