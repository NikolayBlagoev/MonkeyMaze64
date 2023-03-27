#version 460

// Albedo data
layout(location = 3) uniform sampler2D albedoTex;
layout(location = 4) uniform bool hasAlbedo;
layout(location = 5) uniform vec3 defaultAlbedo;

// Normal map data
layout(location = 6) uniform sampler2D normalTex;
layout(location = 7) uniform bool hasNormal;

// Metallic data
layout(location = 8) uniform sampler2D metallicTex;
layout(location = 9) uniform bool hasMetallic;
layout(location = 10) uniform float defaultMetallic;

// Roughness data
layout(location = 11) uniform sampler2D roughnessTex;
layout(location = 12) uniform bool hasRoughness;
layout(location = 13) uniform float defaultRoughness;

// AO map data
layout(location = 14) uniform sampler2D aoTex;
layout(location = 15) uniform bool hasAO;
layout(location = 16) uniform float defaultAO;

// Input from vertex shader
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

// G-buffer output
layout(location = 0) out vec3 gPosition;    // Position buffer
layout(location = 1) out vec3 gNormal;      // Normal buffer
layout(location = 2) out vec4 gAlbedo;      // Albedo buffer (fourth component is specular shininess)
layout(location = 3) out vec3 gMaterial;    // Red channel is metallic, green channel is roughness, blue channel is AO

void main() {
    gPosition   = fragPos;
    
    // TODO: Process normal map
    gNormal     = normalize(fragNormal);

    // Albedo
    if (hasAlbedo)  { gAlbedo = vec4(texture(albedoTex, fragTexCoord).rgb, 10.0); } // TODO: Extract shininess from texture
    else            { gAlbedo = vec4(defaultAlbedo, 10.0); }

    // Metallic
    if (hasMetallic)    { gMaterial.r = texture(metallicTex, fragTexCoord).r; }
    else                { gMaterial.r = defaultMetallic; }

    // Roughness
    if (hasRoughness)   { gMaterial.g = texture(roughnessTex, fragTexCoord).r; }
    else                { gMaterial.g = defaultRoughness; }

    // AO
    if (hasAO)  { gMaterial.b = texture(aoTex, fragTexCoord).r; }
    else        { gMaterial.b = defaultAO; }
}
