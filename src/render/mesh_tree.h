#ifndef _MESH_TREE_H_
#define _MESH_TREE_H_


#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <gameplay/enemy_camera.h>
#include <render/lighting.h>
#include <render/mesh.h>
#include <utils/hitbox.hpp>

#include <filesystem>
#include <vector>
#include <optional>
#include <string>
#include <unordered_map>

struct MeshTransform {
    glm::vec3 translate;
    glm::vec4 selfRotate; // ROTATE AROUND AXIS
    glm::vec4 rotateParent;
    glm::vec3 scale;
};

class MeshTree : public std::enable_shared_from_this<MeshTree> {
public:
    MeshTree(std::string tag,
             const std::optional<HitBox>& maybeHitBox,
             GPUMesh* model         = nullptr,
             glm::vec3 off          = glm::vec3(0.0f), 
             glm::vec4 rots         = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
             glm::vec4 rotp         = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
             glm::vec3 scl          = glm::vec3(1.0f));
    ~MeshTree();

    // Collision detection
    static MeshTree* collidesWith(MeshTree* root, MeshTree* toCheck);
    bool tryTranslation(glm::vec3 translation, MeshTree* root);

    // Mesh management
    void clean(LightManager& lmngr);
    void addChild(std::shared_ptr<MeshTree> child);
    glm::mat4 modelMatrix() const;

    // IF THIS GOES SOMEWHERE ELSE MY APPLICATION WILL NOT RUN!!?!?!? IDK WHY?!?!?! C++ PLEASE! PLEASE!!! WHY?!?!?!
    std::shared_ptr<EnemyCamera> enemyCam;
public:
    // Intrinsic properties
    std::string tag;
    MeshTransform transform;
    GPUMesh* mesh { nullptr };
    std::optional<HitBox> hitBox;

    // Tree hierarchy management
    bool is_root = false;
    std::weak_ptr<MeshTree> parent;
    std::vector<std::weak_ptr<MeshTree>> children;
    
    // External objects manipulated by node
    AreaLight*  al { nullptr };
    PointLight* pl { nullptr };
    
private:
    HitBox getTransformedHitBox();
    glm::vec3 getTransformedHitBoxMiddle();
    bool collide(MeshTree* other);

};

class MemoryManager {
public:
    MemoryManager(){};

    static void addEl(MeshTree* el){ objs[el] = std::shared_ptr<MeshTree> (el); }
    static void removeEl(MeshTree* el) { objs.erase(el); };

    static std::unordered_map<MeshTree*, std::shared_ptr<MeshTree>> objs;
};

#endif
