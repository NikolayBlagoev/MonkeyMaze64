#version 460

layout(location = 0) in vec4 fragParticleColor;

layout(location = 0) out vec4 fragColor;

void main() { fragColor = fragParticleColor; }
