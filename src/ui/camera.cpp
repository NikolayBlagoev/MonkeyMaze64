#include "camera.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
DISABLE_WARNINGS_POP()

Camera::Camera(Window* pWindow, const RenderConfig& renderConfig)
    : Camera(pWindow, renderConfig, glm::vec3(0), glm::vec3(0, 0, -1))
{
}

Camera::Camera(Window* pWindow, const RenderConfig& renderConfig, const glm::vec3& pos, const glm::vec3& forward)
    : m_position(pos)
    , m_forward(glm::normalize(forward))
    , m_pWindow(pWindow)
    , m_renderConfig(renderConfig)
{
}

void Camera::setUserInteraction(bool enabled)
{
    m_userInteraction = enabled;
}

glm::vec3 Camera::cameraPos() const
{
    return m_position;
}

glm::mat4 Camera::viewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_forward, m_up);
}

void Camera::rotateX(float angle)
{
    const glm::vec3 horAxis = glm::cross(s_yAxis, m_forward);

    m_forward = glm::normalize(glm::angleAxis(angle, horAxis) * m_forward);
    m_up = glm::normalize(glm::cross(m_forward, horAxis));
}

void Camera::rotateY(float angle)
{
    const glm::vec3 horAxis = glm::cross(s_yAxis, m_forward);

    m_forward = glm::normalize(glm::angleAxis(angle, s_yAxis) * m_forward);
    m_up = glm::normalize(glm::cross(m_forward, horAxis));
}

void Camera::updateInput() {
    if (m_userInteraction) {
        const glm::vec3 right = glm::normalize(glm::cross(m_forward, m_up));

        // Forward, backward and strafe
        if (m_pWindow->isKeyPressed(GLFW_KEY_A))
            m_position -= m_renderConfig.moveSpeed * right;
        if (m_pWindow->isKeyPressed(GLFW_KEY_D))
            m_position += m_renderConfig.moveSpeed * right;
        if (m_pWindow->isKeyPressed(GLFW_KEY_W))
            m_position += m_renderConfig.moveSpeed * m_forward;
        if (m_pWindow->isKeyPressed(GLFW_KEY_S))
            m_position -= m_renderConfig.moveSpeed * m_forward;

        // Up and down
        if (!m_renderConfig.constrainVertical) {
            if (m_pWindow->isKeyPressed(GLFW_KEY_SPACE)) { m_position += m_renderConfig.moveSpeed * m_up; }
            if (m_pWindow->isKeyPressed(GLFW_KEY_C)) { m_position -= m_renderConfig.moveSpeed * m_up; }
        }

        // Mouse movement
        const glm::dvec2 cursorPos = m_pWindow->getCursorPos();
        glm::vec2 delta = m_renderConfig.lookSpeed * glm::vec2(m_prevCursorPos - cursorPos);
        // Invert controls
        if (m_renderConfig.invertControls)
            delta *= -1;

        m_prevCursorPos = cursorPos;
        if (m_pWindow->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            rotateY(delta.x);

            if (!m_renderConfig.constrainVertical)
                rotateX(delta.y);
        }

    }
    else {
        m_prevCursorPos = m_pWindow->getCursorPos();
    }
}

void Camera::updateInput(Scene& scene, int idx) {
    if (m_userInteraction) {
        glm::vec3 right = glm::normalize(glm::cross(m_forward, m_up)) * m_renderConfig.moveSpeed;
        glm::vec3 forward = m_forward * m_renderConfig.moveSpeed;
        glm::vec3 up = m_up * m_renderConfig.moveSpeed;

        // Eliminate y for forwards and backwards
        right.y = 0.0f;
        forward.y = 0.0f;

        // Forward, backward and strafe
        if (m_pWindow->isKeyPressed(GLFW_KEY_A)) {
            if (scene.checkTranslationValid(idx, -right))
                m_position -= right;
        }
        if (m_pWindow->isKeyPressed(GLFW_KEY_D)) {
            if (scene.checkTranslationValid(idx, right))
                m_position += right;
        }
        if (m_pWindow->isKeyPressed(GLFW_KEY_W)) {
            if (scene.checkTranslationValid(idx, forward))
                m_position += forward;
        }
        if (m_pWindow->isKeyPressed(GLFW_KEY_S)) {
            if (scene.checkTranslationValid(idx, -forward))
                m_position -= forward;
        }

        // Up and down
        if (!m_renderConfig.constrainVertical) {
            if (m_pWindow->isKeyPressed(GLFW_KEY_SPACE)) {
                if (scene.checkTranslationValid(idx, up))
                    m_position += up;
            }
            if (m_pWindow->isKeyPressed(GLFW_KEY_C)) {
                if (scene.checkTranslationValid(idx, -up))
                    m_position -= up;
            }
        }

        // Mouse movement
        const glm::dvec2 cursorPos = m_pWindow->getCursorPos();
        glm::vec2 delta = m_renderConfig.lookSpeed * glm::vec2(m_prevCursorPos - cursorPos);
        // Invert controls
        if (m_renderConfig.invertControls)
            delta *= -1;

        m_prevCursorPos = cursorPos;
        if (m_pWindow->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {

            glm::vec3 objectPos = scene.m_transformParams[idx].translate;
            glm::vec3 vecToObject = objectPos - m_position;
            // move to object
            m_position = objectPos;

            // rotate
            rotateY(delta.x);
            //TODO: FIX THIS

            // scene.m_transformParams[idx].rotate.y += glm::degrees(delta.x);

            // if (!m_renderConfig.constrainVertical) {
            //     rotateX(delta.y);
            //     scene.m_transformParams[idx].rotate.x -= glm::degrees(delta.y);
            // }

            // move back
            m_position -= glm::normalize(m_forward) * glm::length(vecToObject);
        }
    }
    else {
        m_prevCursorPos = m_pWindow->getCursorPos();
    }
}
