#ifndef _DEFERRED_H_
#define _DEFERRED_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
DISABLE_WARNINGS_POP()

#include <render/scene.h>

// TODO: Adapt to resize framebuffer sizes when window size changes
class DeferredRenderer {
public:
    DeferredRenderer(Scene& scene, int32_t screenWidth, int32_t screenHeight);
    ~DeferredRenderer();



private:
    static constexpr GLuint INVALID = 0xFFFFFFFF;

    GLuint gBuffer;     // Framebuffer ID for the G-buffer
    GLuint rboDepth;    // Depth buffer

    // Textures for storing G-buffer attributes
    // Must match attributes used by fragment shader(s)
    GLuint positionTex;
    GLuint normalTex;
    GLuint albedoTex;

    Scene& m_scene;
};

#endif
