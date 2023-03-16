#version 460
in vec4 fragPos;

layout(location = 1) uniform vec3 lightPos;
layout(location = 2) uniform float farPlane;

void main() {
    // Get distance between fragment and light source
    float lightDistance = length(fragPos.xyz - lightPos);
    
    // Map to [0:1] range by dividing by farPlane
    lightDistance = lightDistance / farPlane;
    
    // Write this as modified depth
    gl_FragDepth = lightDistance;
}
