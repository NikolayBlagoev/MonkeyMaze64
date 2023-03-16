#version 460

layout(location = 0) uniform vec3 lightPos;
layout(location = 1) uniform float farPlane;

in vec4 fragPos;

void main() {
    // Get distance between fragment and light source
    float lightDistance = length(fragPos.xyz - lightPos);
    
    // Map to [0:1] range by dividing by farPlane
    lightDistance = lightDistance / farPlane;
    
    // Write this as modified depth
    gl_FragDepth = lightDistance;
}
