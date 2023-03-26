#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <render/config.h>
#include <render/scene.h>

// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/window.h>

class Camera {
public:
    Camera(Window* pWindow, const RenderConfig& m_renderConfig);
    Camera(Window* pWindow, const RenderConfig& m_renderConfig,
           const glm::vec3& position, const glm::vec3& forward);

    // Update camera position freely
    void updateInput();
    // Update camera position with respect to object
    void updateInput(Scene& scene, size_t idx);

    void setUserInteraction(bool enabled);

    glm::vec3 cameraPos() const;
    glm::mat4 viewMatrix() const;

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
