#version 460

// Albedo data
layout(location = 3) uniform sampler2D albedoTex;
layout(location = 4) uniform bool hasAlbedo;
layout(location = 5) uniform vec4 defaultAlbedo;

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

// Displacement map data
layout(location = 17) uniform sampler2D displacementTex;
layout(location = 18) uniform bool hasDisplacement;
layout(location = 19) uniform bool displacementIsHeight;
layout(location = 20) uniform float heightScale;    // Controls strength of parallax effect
layout(location = 21) uniform float minDepthLayers; // Min number of samples to use to approximate depth at intersection point
layout(location = 22) uniform float maxDepthLayers; // Max number of samples to use to approximate depth at intersection point

// Camera position
layout(location = 23) uniform vec3 cameraPos;

// Input from vertex shader
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in mat3 tbn;          // TBN matrix for normal and/or displacement map(s) transformation(s)

// G-buffer output
layout(location = 0) out vec3 gPosition;    // Position buffer
layout(location = 1) out vec3 gNormal;      // Normal buffer
layout(location = 2) out vec4 gAlbedo;      // Albedo buffer
layout(location = 3) out vec3 gMaterial;    // Red channel is metallic, green channel is roughness, blue channel is AO

vec2 parallaxMapping() {
    vec3 fragToCamera       = normalize(tbn * (cameraPos - fragPos));                                                   // This vector is in tangent space
    float numDepthLayers    = mix(maxDepthLayers, minDepthLayers, max(dot(vec3(0.0, 0.0, 1.0), fragToCamera), 0.0));    // Scale number of samples based on how sharp the view angle on the surface is (sharper angle => closer to maxLayers)
    float layerDepth        = 1.0 / numDepthLayers;
    float currentLayerDepth = 0.0;
    vec2 surfaceToIncident  = fragToCamera.xy * heightScale;
    vec2 deltaTexCoords     = surfaceToIncident / numDepthLayers;

    // Continually sample until we find a depth (in the depth map) below the theoretical real depth we are currently testing
    vec2 currentTexCoords       = fragTexCoord;
    float currentDepthMapValue  = displacementIsHeight ? 1.0 - texture(displacementTex, currentTexCoords).r : texture(displacementTex, currentTexCoords).r;    
    while (currentLayerDepth < currentDepthMapValue) {
        currentTexCoords        -= deltaTexCoords;                             // Shift texture coordinates along opposite direction of P
        currentDepthMapValue    = displacementIsHeight ? 1.0 - texture(displacementTex, currentTexCoords).r : texture(displacementTex, currentTexCoords).r; // Get depthmap value at current texture coordinates
        currentLayerDepth       += layerDepth;                                // Get depth of next layer
    }

    // Linearly interpolate between post hit depth and the depth value of the previous sample
    vec2 prevTexCoords  = currentTexCoords -= deltaTexCoords;
    float beforeDepth   = (displacementIsHeight ? 1.0 - texture(displacementTex, currentTexCoords).r : texture(displacementTex, currentTexCoords).r) - currentLayerDepth + layerDepth;
    float afterDepth    = currentDepthMapValue - currentLayerDepth;
    float weight        = afterDepth / (afterDepth - beforeDepth);
    return (prevTexCoords * weight) + (currentTexCoords * (1.0 - weight));
} 

void main() {
    gPosition = fragPos;

    // Transform texture coords if height map is present
    vec2 finalTexCoords;
    if (hasDisplacement) {
        finalTexCoords = parallaxMapping();
        if (finalTexCoords.x < 0.0 || finalTexCoords.x > 1.0 || // If tex coords are offset outside the [0, 1] range, discard this fragment
            finalTexCoords.y < 0.0 || finalTexCoords.y > 1.0) { discard; }

    } else { finalTexCoords = fragTexCoord; }
    
    // Normal
    if (hasNormal) { 
        gNormal = texture(normalTex, finalTexCoords).rgb;
        gNormal = gNormal * 2.0 - 1.0;   
        gNormal = normalize(tbn * gNormal); 
    } else { gNormal = normalize(fragNormal); }

    // Albedo
    if (hasAlbedo)  { gAlbedo = texture(albedoTex, finalTexCoords); }
    else            { gAlbedo = defaultAlbedo; }

    // Metallic
    if (hasMetallic)    { gMaterial.r = texture(metallicTex, finalTexCoords).r; }
    else                { gMaterial.r = defaultMetallic; }

    // Roughness
    if (hasRoughness)   { gMaterial.g = texture(roughnessTex, finalTexCoords).r; }
    else                { gMaterial.g = defaultRoughness; }

    // AO
    if (hasAO)  { gMaterial.b = texture(aoTex, finalTexCoords).r; }
    else        { gMaterial.b = defaultAO; }
}
