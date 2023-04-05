#include "mesh_tree.h"

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

#include <iostream>

std::unordered_map<MeshTree*, std::shared_ptr<MeshTree>> MemoryManager::objs;

MeshTree::MeshTree(std::string tag, const std::optional<HitBox>& maybeHitBox, GPUMesh* model,
                   glm::vec3 off, glm::vec4 rots, glm::vec4 rotp, glm::vec3 scl) {
    this->transform = {off, rots, rotp, scl};
    this->tag       = tag;
    this->mesh      = model;
    this->hitBox    = maybeHitBox;
}

MeshTree::~MeshTree() { std::cout<< "Destructor called for MeshTree with tag: " << tag << std::endl; }

void MeshTree::addChild(std::shared_ptr<MeshTree> child){
    child.get()->parent = std::weak_ptr<MeshTree>(shared_from_this());
    this->children.push_back(child);
}

void MeshTree::transformExternal() {
    // Transform objects managed by this node
    glm::mat4 currTransform = modelMatrix(false);
    if (al != nullptr) {
        al->position        = currTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        al->externalForward = glm::normalize(currTransform * glm::vec4(-1.0f, 0.0f, 0.0f, 1.0f));
    }
    if (pl != nullptr) { pl->position = currTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); }
    if (particleEmitter != nullptr) { particleEmitter->m_position = currTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); }

    // Transform objects managed by children
    for (std::weak_ptr<MeshTree> child : children) { if (!child.expired()) { child.lock().get()->transformExternal(); } }
}

glm::mat4 MeshTree::modelMatrix(bool includeScale) const {
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

    // Translate
    currTransform = glm::translate(currTransform, transform.translate);

    // Rotate
    currTransform = glm::rotate(currTransform, glm::radians(transform.selfRotate.w), glm::vec3(transform.selfRotate.x, transform.selfRotate.y, transform.selfRotate.z));

    // Scale
    if (includeScale) { currTransform = glm::scale(currTransform, transform.scale); }

    return currTransform;
}

HitBox MeshTree::getTransformedHitBox() {
    if (!hitBox.has_value()) {
        // Massively far away to ensure no collision
        glm::vec3 massivelyFar(std::numeric_limits<float>::max());
        std::array<glm::vec3, 8> massivelyFarPoints;
        std::fill(massivelyFarPoints.begin(), massivelyFarPoints.end(), massivelyFar);
        return { .allowCollision = false, .points = massivelyFarPoints };
    } 

    glm::mat4 modelMatrix   = this->modelMatrix();
    HitBox transformed      = this->hitBox.value();
    for (glm::vec3& point : transformed.points) { point = modelMatrix * glm::vec4(point, 1.0f); }
    return transformed;
}

glm::vec3 MeshTree::getTransformedHitBoxMiddle() { 
    if (!hitBox.has_value()) { return glm::vec3(std::numeric_limits<float>::max()); } // Massively far away to ensure no collision
    return modelMatrix() * glm::vec4(this->hitBox.value().getMiddle(), 1.0f);
}

bool MeshTree::collide(MeshTree *other) {
    if (other == nullptr            || other == this ||
        !this->hitBox.has_value()   || !other->hitBox.has_value()) { return false; }
    return ((this->hitBox.value().allowCollision && other->hitBox.value().allowCollision) &&
             this->getTransformedHitBox().collides(other->getTransformedHitBox()));
}

MeshTree* MeshTree::collidesWith(MeshTree* root, MeshTree* toCheck) {
    if (root->collide(toCheck)) { return root; }

    for (size_t childIdx = 0; childIdx < root->children.size(); childIdx++) {
        std::weak_ptr<MeshTree> child = root->children.at(childIdx);
        
        // Remove child if it has been free'd
        if (child.expired()) { 
            root->children.erase(root->children.begin() + childIdx);
            childIdx--;
            continue;
        }

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

void MeshTree::clean(LightManager& lmngr, ParticleEmitterManager& particleEmitterManager){
    // Destroy all children
    for (size_t childIdx = 0; childIdx < children.size(); childIdx++) {
        if (!children[childIdx].expired()) {
            MeshTree* child = children[childIdx].lock().get();
            MemoryManager::removeEl(child);
        }
    }

    // Destroy all external objects (if any)
    if (al != nullptr)              { lmngr.removeByReference(al); }
    if (pl != nullptr)              { lmngr.removeByReference(pl); }
    if (particleEmitter != nullptr) { particleEmitterManager.removeByReference(particleEmitter); }
}
