#version 460

struct Particle {
    vec3 position;
    vec4 color;
    float size;
};

layout(location = 0) uniform mat4 viewProjection;

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in Particle particleData;

layout(location = 0) out vec4 particleColor;

void main() {
    particleColor   = particleData.color;
    gl_Position     = viewProjection * vec4((particleData.position * particleData.size) + vertexPos, 1.0);
}
