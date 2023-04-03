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
#include <camera.h>
#include "lighting.h"
#include <string.h>
#include <unordered_map>

struct MeshTransform {
    glm::vec3 translate;
    glm::vec4 selfRotate; // ROTATE AROUND AXIS
    glm::vec4 rotateParent;
    glm::vec3 scale;
};

class MeshTree : public std::enable_shared_from_this<MeshTree> {
public:
    MeshTree(std::string tag, Mesh* msh, glm::vec3 off, glm::vec4 rots, glm::vec4 rotp, glm::vec3 scl)
        : MeshTree(tag, msh, off, rots, rotp, scl, true) {}
    MeshTree(std::string tag, Mesh* msh, glm::vec3 off, glm::vec4 rots, glm::vec4 rotp, glm::vec3 scl, bool allowCollision);
    MeshTree(std::string tag);
    ~MeshTree();
    MeshTree(std::string tag, Mesh* msh)
        : MeshTree(tag, msh, true) {}
    MeshTree(std::string tag, Mesh* msh, bool allowCollision);
    void clean(LightManager& lmngr);
    void addChild(std::shared_ptr<MeshTree> child);
    glm::mat4 modelMatrix() const;
    HitBox getTransformedHitBox();
    glm::vec3 getTransformedHitBoxMiddle();
    bool collide(MeshTree* other);
    
    bool tryTranslation(glm::vec3 translation, MeshTree* root);

public:
    GPUMesh* mesh;
    std::string tag;
    MeshTransform transform;
    HitBox hitBox;
    // std::shared_ptr<CameraObj> camera;
    // MeshTree* parent { nullptr };
    bool is_root = false;
    std::weak_ptr<MeshTree> parent;
    // std::shared_ptr<MeshTree> self;
    std::vector<std::weak_ptr<MeshTree>> children;
    // std::shared_ptr<glm::vec4> selfRotate;
    // std::shared_ptr<glm::vec4> rotateParent;
    // std::shared_ptr<glm::vec3> translate;
    // std::shared_ptr<glm::vec3> scale;
    AreaLight* al { nullptr };
};

class MemoryManager {
    public:
    MemoryManager(){

    };
    static std::unordered_map<MeshTree*, std::shared_ptr<MeshTree>> objs;
    static void addEl(MeshTree* el){
        objs[el] = std::shared_ptr<MeshTree> (el);
    }
    static void removeEl(MeshTree* el){
        objs.erase(el);
    };
    
};

#endif
