#version 460

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(binding = 0) buffer pointLights { PointLight pointLightsData[]; };

layout(location = 3) uniform sampler2D colorMap;
layout(location = 4) uniform bool hasTexCoords;

// Camera position
layout(location = 5) uniform vec3 cameraPos;

// Lighting parameter(s)
layout(location = 6) uniform float objectShininess = 10.0;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;

vec3 lambertianDiffuse(vec3 lightColor, vec3 lightPos, vec3 materialProps) {
    vec3 lightDir = normalize(lightPos - fragPosition);
    return dot(lightDir, fragNormal) * lightColor * materialProps;
}

vec3 phongSpecular(vec3 lightColor, vec3 lightPos, vec3 materialProps) {
    vec3 lightToSurface     = normalize(fragPosition - lightPos);
    vec3 surfaceToCamera    = normalize(cameraPos - fragPosition);
    vec3 reflection         = reflect(lightToSurface, fragNormal);

    float lightNormalDot    = dot(-lightToSurface, fragNormal);
    float reflectionViewDot = dot(reflection, surfaceToCamera);
    return  lightNormalDot > 0.0 && reflectionViewDot > 0.0 ?
            materialProps * pow(reflectionViewDot, objectShininess) * lightColor :
            vec3(0.0, 0.0, 0.0);
}

void main() {
    const vec3 materialProps    = hasTexCoords ? texture(colorMap, fragTexCoord).rgb : vec3(1.0, 1.0, 1.0);

    // Accumulate lighting from all sources
    fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    for (uint lightIdx = 0U; lightIdx < pointLightsData.length(); lightIdx++) {
        PointLight light    = pointLightsData[lightIdx];
        vec3 lightColor     = light.color.rgb;
        vec3 lightPosition  = light.position.xyz;
        fragColor.rgb += lambertianDiffuse(lightColor, lightPosition, materialProps) +
                         phongSpecular(lightColor, lightPosition, materialProps);
    }
}
