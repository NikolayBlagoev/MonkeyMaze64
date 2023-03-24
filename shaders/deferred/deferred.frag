#version 460

// Texture data
layout(location = 3) uniform sampler2D colorMap;
layout(location = 4) uniform bool hasTexCoords;

// Input from vertex shader
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

// G-buffer output
layout(location = 0) out vec3 gPosition;    // Position buffer
layout(location = 1) out vec3 gNormal;      // Normal buffer
layout(location = 2) out vec4 gAlbedo;      // Albedo buffer (fourth component is specular shininess)

void main() {
    gPosition   = fragPos;
    gNormal     = normalize(fragNormal);
    if (hasTexCoords) { gAlbedo = vec4(texture(colorMap, fragTexCoord).rgb, 10.0); } // TODO: Extract shininess from texture
    else { gAlbedo = vec4(1.0, 1.0, 1.0, 10.0); } // Default white material colour
}
