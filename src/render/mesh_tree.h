#ifndef _MESH_TREE_H_
#define _MESH_TREE_H_

#include "mesh.h"
#include "utils/hitBox.h"
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <filesystem>
#include <vector>

struct MeshTransform {
    glm::vec3 translate;
    glm::vec4 selfRotate; // ROTATE AROUND AXIS
    glm::vec4 rotateParent;
    glm::vec3 scale;
};

class MeshTree {
public:
   
    MeshTree(Mesh* msh, glm::vec3 off, glm::vec4 rots, glm::vec4 rotp, glm::vec3 scl)
        : MeshTree(msh, off, rots, rotp, scl, true) {}
    MeshTree(Mesh* msh, glm::vec3 off, glm::vec4 rots, glm::vec4 rotp, glm::vec3 scl, bool allowCollision);
    MeshTree();
    MeshTree(Mesh* msh)
        : MeshTree(msh, true) {}
    MeshTree(Mesh* msh, bool allowCollision);
    
    void addChild(MeshTree* child);

    glm::mat4 modelMatrix();

    HitBox getTransformedHitBox();
    glm::vec3 getTransformedHitBoxMiddle();

    bool collide(MeshTree* other);

    bool tryTranslation(glm::vec3 translation);

public:
    GPUMesh* mesh;
    MeshTransform transform;
    HitBox hitBox;
    MeshTree* parent { nullptr };
    std::vector<MeshTree*> children;
};

#endif
