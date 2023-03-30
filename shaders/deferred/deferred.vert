#version 460

// Transformation matrices
layout(location = 0) uniform mat4 mvpMatrix;
layout(location = 1) uniform mat4 modelMatrix;
layout(location = 2) uniform mat3 normalModelMatrix; // Normals should be transformed differently than positions (https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html)

// Normal map data
layout(location = 6) uniform sampler2D normalTex;
layout(location = 7) uniform bool hasNormal;

// Displacement map data
layout(location = 17) uniform sampler2D displacementTex;
layout(location = 18) uniform bool hasDisplacement;
layout(location = 19) uniform bool displacementIsHeight;

// Must match vertex properties definition
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

// Must match vertex properties definition
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord; 
layout(location = 3) out mat3 tbn;          // TBN matrix for normal map transformation

void main() {
    // Screen-space position
    gl_Position = mvpMatrix * vec4(position, 1.0);

    // Compute TBN if needed for either normal or displacement map
    if (hasNormal || hasDisplacement) { 
        vec3 tangentCol     = normalize(vec3(modelMatrix * vec4(tangent,   0.0)));
        vec3 bitangentCol   = normalize(vec3(modelMatrix * vec4(bitangent, 0.0)));
        vec3 normalCol      = normalize(vec3(modelMatrix * vec4(normal,    0.0)));
        tbn                 = mat3(tangentCol, bitangentCol, normalCol);
    }

    // Use fragment normal if a normal map is not provided
    if (!hasNormal) { fragNormal = normalModelMatrix * normal; }
    
    // World-space position and texture coords
    fragPos         = (modelMatrix * vec4(position, 1.0)).xyz;
    fragTexCoord    = texCoord;
}
