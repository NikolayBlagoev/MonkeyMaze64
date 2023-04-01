#ifndef _RENDER_UTILS_H_
#define _RENDER_UTILS_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
DISABLE_WARNINGS_POP()

namespace utils {
    // Screen-filling quad to render to
    static GLuint quadVAO = 0U;
    static GLuint quadVBO;

    static void renderQuad(const float ensmall) {
        if (quadVAO == 0U || true) {
            const float arithmeticStuff = (1.f-ensmall);

            
            std::array<float, 20UL> quadVertices = {
                // Positions        // Texture coords
                
                // -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                // -1.0f, -1.f, 0.0f, 0.0f, 0.0f,
                // 1.f,  1.0f, 0.0f, 1.0f, 1.0f,
                // 1.f, -1.f, 0.0f, 1.0f, 0.0f,
                
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f + arithmeticStuff,
                1.0f ,  1.0f, 0.0f, 1.0f - arithmeticStuff, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f - arithmeticStuff, 0.0f + arithmeticStuff,
            };
//                  -1.0f,  1.0f, 0.0f, 0.0f + arithmeticStuff, 1.0f - arithmeticStuff,
//                 -1.0f, 1.0f - 2.f*ensmall, 0.0f, 0.0f + arithmeticStuff, 0.0f + arithmeticStuff,
//                 -1.0f + 2.f*ensmall,  1.0f, 0.0f, 1.0f - arithmeticStuff, 1.0f - arithmeticStuff,
//                 -1.0f + 2.f*ensmall, 1.0f - 2.f*ensmall, 0.0f, 1.0f - arithmeticStuff, 0.0f + arithmeticStuff,
            // Set up plane VAO
            glCreateVertexArrays(1, &quadVAO);
            glCreateBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

#endif 
