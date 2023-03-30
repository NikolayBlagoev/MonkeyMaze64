#version 460

layout(location = 0) uniform mat4 viewProjection;

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 particlePosition;
layout(location = 2) in vec4 particleColor;
layout(location = 3) in float particleSize;

layout(location = 0) out vec4 fragParticleColor;

void main() {
    fragParticleColor   = particleColor;
    gl_Position         = viewProjection * vec4(particlePosition + (particleSize * vertexPos), 1.0);
}
