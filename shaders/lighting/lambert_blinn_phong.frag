#version 460

struct PointLight {
    vec4 position;
    vec4 color;
};

struct AreaLight {
    vec4 position;
    vec4 color;
    mat4 viewProjection;
    float falloff;
};

// SSBOs
layout(binding = 0) buffer pointLights { PointLight pointLightsData[]; };
layout(binding = 1) buffer areaLights { AreaLight areaLightsData[]; };

// G-buffer data
layout(location = 0) uniform sampler2D gPosition;
layout(location = 1) uniform sampler2D gNormal;
layout(location = 2) uniform sampler2D gAlbedo;
layout(location = 3) uniform sampler2D gMaterial;

// Camera position
layout(location = 4) uniform vec3 cameraPos;

// Lighting and shading parameter(s)
layout(location = 5) uniform float objectShininess = 10.0; // TODO: Remove
layout(location = 6) uniform float shadowFarPlane;

// Shadow map array(s)
layout(location = 7) uniform samplerCubeArrayShadow pointShadowTexArr;
layout(location = 8) uniform sampler2DArrayShadow areaShadowTexArr;

// Quad texture to use with G-buffer
layout(location = 0) in vec2 bufferCoords;

// Output
layout(location = 0) out vec4 fragColor;

/*****************************************************************************************************/

// @param sampleCoord: Coordinates of fragment sample
// @param lightIdx: Index of the point light in the SSBO/shadow map
float samplePointShadow(vec3 sampleCoord, uint lightIdx) {
    vec3 lightToSample      = sampleCoord - pointLightsData[lightIdx].position.xyz;
    float clippedDistance   = length(lightToSample) / shadowFarPlane;
    vec4 texIdx             = vec4(lightToSample, lightIdx);

    const float EPSILON = 1e-3 * exp(4 * clippedDistance); // Adaptive epsilon for shadow acne
    return texture(pointShadowTexArr, texIdx, clippedDistance - EPSILON);
}

// @param sampleLightCoord: Homogeneous coordinates of fragment sample tranformed by viewProjection of shadow-casting light
// @param lightIdx: Index of the area light in the SSBO/shadow map
float sampleAreaShadow(vec4 sampleLightCoord, uint lightIdx) {
    // Divide by w because sampleLightCoord are homogeneous coordinates
    sampleLightCoord.xyz /= sampleLightCoord.w;

    // The resulting value is in NDC space (-1 to +1), we transform them to texture space (0 to 1).
    sampleLightCoord.xyz = sampleLightCoord.xyz * 0.5 + 0.5;

    // Add slight epsilon to fragment depth value used for comparison
    const float EPSILON = 1e-3;
    sampleLightCoord.z -= EPSILON;

    // Shadow map value from the corresponding shadow map position ()
    vec4 texcoord;
    texcoord.xyw    = sampleLightCoord.xyz;
    texcoord.z      = lightIdx;
    return texture(areaShadowTexArr, texcoord);
}

/*****************************************************************************************************/

vec3 lambertianDiffuse(vec3 fragPos, vec3 fragNormal, vec3 fragAlbedo,
                       vec3 lightColor, vec3 lightPos) {
    vec3 lightDir = normalize(lightPos - fragPos);
    return dot(lightDir, fragNormal) * lightColor * fragAlbedo;
}

vec3 blinnPhongSpecular(vec3 fragPos, vec3 fragNormal, vec3 fragAlbedo,
                        vec3 lightColor, vec3 lightPos) {
    vec3 surfaceToLight     = normalize(lightPos - fragPos);
    vec3 surfaceToCamera    = normalize(cameraPos - fragPos);
    vec3 vectorsSum         = surfaceToLight + surfaceToCamera;
    vec3 halfway            = vectorsSum / length(vectorsSum);

    float lightNormalDot    = dot(surfaceToLight, fragNormal);
    float normalHalfwayDot  = dot(fragNormal, halfway);
    return lightNormalDot > 0.0 && normalHalfwayDot > 0.0 ?
           fragAlbedo * pow(normalHalfwayDot, objectShininess) * lightColor :
           vec3(0.0, 0.0, 0.0);
}


vec3 ambient(vec3 lightColor) {

    return 0.1 * lightColor;
}
/*****************************************************************************************************/

void main() {
    // Extract values from G-buffer
    vec3 fragPos    = texture(gPosition, bufferCoords).xyz;
    vec3 fragNormal = texture(gNormal, bufferCoords).xyz;
    vec3 fragAlbedo = texture(gAlbedo, bufferCoords).rgb;

    fragColor = vec4(0.0, 0.0, 0.0, 1.0);

    // Accumulate lighting from point lights
    for (uint lightIdx = 0U; lightIdx < pointLightsData.length(); lightIdx++) {
        PointLight light    = pointLightsData[lightIdx];
        vec3 lightColor     = light.color.rgb;
        vec3 lightPosition  = light.position.xyz;
        
        float successFraction   = samplePointShadow(fragPos, lightIdx);
        if (successFraction != 0.0) { 
            fragColor.rgb += successFraction * lambertianDiffuse(fragPos, fragNormal, fragAlbedo, lightColor, lightPosition);
            fragColor.rgb += successFraction * blinnPhongSpecular(fragPos, fragNormal, fragAlbedo, lightColor, lightPosition);
        }
    }

    // Accumulate lighting from area lights
    for (uint lightIdx = 0U; lightIdx < areaLightsData.length(); lightIdx++) {
        AreaLight light     = areaLightsData[lightIdx];
        vec3 lightColor     = light.color.rgb;
        vec3 lightPosition  = light.position.xyz;

        vec4 fragLightCoord     = light.viewProjection * vec4(fragPos, 1.0);
        float successFraction   = sampleAreaShadow(fragLightCoord, lightIdx);
        if (successFraction != 0.0) { 
            fragColor.rgb += successFraction * lambertianDiffuse(fragPos, fragNormal, fragAlbedo, lightColor, lightPosition);
            fragColor.rgb += successFraction * blinnPhongSpecular(fragPos, fragNormal, fragAlbedo, lightColor, lightPosition);
        }
    }
    fragColor.rgb+=ambient(vec3(1.0,1.0,1.0));
}
