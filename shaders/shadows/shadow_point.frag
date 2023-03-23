#version 460

layout(location = 2) uniform vec3 lightPos;
layout(location = 3) uniform float farPlane;

layout(location = 0) in vec3 fragPos;

void main() {
    // Get distance between vertex and light source
    float lightDistance = length(fragPos - lightPos);
    
    // Divide by far plane to get value in range [0:1]
    lightDistance /= farPlane;
    
    // Write this as output (modified depth)
    gl_FragDepth = lightDistance;
}
