#version 460

layout(location = 0) uniform vec4 pos; // Screen-space position

void main() {
    gl_Position = pos;
}
