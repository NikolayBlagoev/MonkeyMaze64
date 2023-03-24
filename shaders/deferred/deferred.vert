#version 460

layout(location = 0) uniform mat4 mvpMatrix;
layout(location = 1) uniform mat4 modelMatrix;
layout(location = 2) uniform mat3 normalModelMatrix; // Normals should be transformed differently than positions (https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html)

// Must match vertex properties definition
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

// Must match vertex properties definition
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1.0);

    fragPos         = (modelMatrix * vec4(position, 1.0)).xyz;
    fragNormal      = normalModelMatrix * normal;
    fragTexCoord    = texCoord;
}
