#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <render/config.h>

// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/window.h>
#include "render/mesh_tree.h"

class Camera {
public:
    Camera(Window* pWindow, const RenderConfig& m_renderConfig);
    Camera(Window* pWindow, const RenderConfig& m_renderConfig, const glm::vec3& position, const glm::vec3& forward);

    void updateInput();
    void updateInput(MeshTree* mesh, MeshTree* root) { updateInput(mesh, root, glm::vec3(0.0f)); }
    void updateInput(MeshTree* mesh, MeshTree* root, glm::vec3 meshMiddleOffset);
    void setUserInteraction(bool enabled);

    glm::vec3 cameraPos() const;
    glm::vec3 topDownPos() const;
    glm::mat4 viewMatrix() const;
    glm::mat4 topDownViewMatrix() const;

    glm::vec3 getForward() const            { return m_forward; }
    void setForward(glm::vec3 newForward)   { m_forward = newForward; }
    void setPosition(glm::vec3 newPosition) { m_position = newPosition; }

    bool canSeePoint(glm::vec3 point);
    bool canSeePoint(glm::vec3 point, float maxDist);

    bool* update;

private:
    void rotateX(float angle);
    void rotateY(float angle);

private:
    static constexpr glm::vec3 s_yAxis { 0, 1, 0 };

    glm::vec3 m_position { 0 };
    glm::vec3 m_forward { 0, 0, -1 };
    glm::vec3 m_up { 0, 1, 0 };

    const Window* m_pWindow;
    const RenderConfig& m_renderConfig;
    bool m_userInteraction { true };
    glm::dvec2 m_prevCursorPos { 0 };
};

#endif
