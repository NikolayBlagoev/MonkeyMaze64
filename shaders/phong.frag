#version 460

struct PointLight {
    vec3 position;
    vec3 colour;
};

layout(binding = 0) buffer pointLights { PointLight pointLightsData[]; };

layout(location = 3) uniform sampler2D colorMap;
layout(location = 4) uniform bool hasTexCoords;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;

void main() {
    const vec3 normal           = normalize(fragNormal);
    const vec3 materialProps    = hasTexCoords ? texture(colorMap, fragTexCoord).rgb : vec3(1.0, 1.0, 1.0);

    fragColor = vec4(pointLightsData[0].colour, 1.0);
}
