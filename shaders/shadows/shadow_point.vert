#version 460

layout(location = 0) uniform mat4 mvp;
layout(location = 1) uniform mat4 model;

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 fragPos;

void main() {
    // Get projection of vertex on current face
    gl_Position = mvp * vec4(position, 1.0);

    // Send world-space coordinate to fragment shader
    fragPos = (model * vec4(position, 1.0)).xyz;
}
