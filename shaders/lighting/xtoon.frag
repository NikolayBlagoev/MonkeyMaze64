#version 460

struct PointLight {
    vec4 position;
    vec4 color;
};

struct AreaLight {
    vec4 position;
    vec4 color;
    mat4 viewProjection;    
    vec4 falloff;           // X-coordinate is constant coefficient, Y-coordinate is linear coefficient, Z-coordinate is quadratic coefficient, W-coordinate is unused
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

// Shader-specific data
layout(location = 9) uniform sampler2D texToon;

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
    // Map sample to light shadowmap's texture coordinate space
    sampleLightCoord.xyz /= sampleLightCoord.w;              // Divide by w because sampleLightCoord are homogeneous coordinates
    sampleLightCoord.xyz = sampleLightCoord.xyz * 0.5 + 0.5; // The resulting value is in NDC space (-1 to +1), we transform them to texture space (0 to 1).

    // Dropoff factor
    vec3 dropoffFactors         = areaLightsData[lightIdx].falloff.xyz;
    float distanceFromCenter    = length(sampleLightCoord.xy - vec2(0.5, 0.5));                 // Distance of sample from center of texture
    float falloffFunction       = dropoffFactors.z * distanceFromCenter * distanceFromCenter +  // Quadratic decay
                                  dropoffFactors.y * distanceFromCenter +
                                  dropoffFactors.x;
    float falloffFactor         = max(0.0, -falloffFunction);                                   // Floor to 0 to prevent negative values  

    // Add slight epsilon to fragment depth value used for comparison
    const float EPSILON = 1e-3;
    sampleLightCoord.z -= EPSILON;

    // Shadow map value from the corresponding shadow map position multiplied by falloff factor
    vec4 texcoord;
    texcoord.xyw    = sampleLightCoord.xyz;
    texcoord.z      = lightIdx;
    return texture(areaShadowTexArr, texcoord) * falloffFactor;
}

/*****************************************************************************************************/

vec3 xToonShading(vec3 fragPos, vec3 fragNormal, vec3 lightPos) {
    float diffuseIntensity  = dot(normalize(fragNormal), normalize(lightPos - fragPos));
    float specularIntensity = dot(normalize(normalize(cameraPos - fragPos) + normalize(lightPos - fragPos)), fragNormal);
    float intensity         = (diffuseIntensity + specularIntensity) / 2.0;
    float cameraToFragDist  = 1.0 - min(1.0 / length(fragPos - cameraPos), 1.0);
    return texture(texToon, vec2(intensity, cameraToFragDist)).rgb;
}

/*****************************************************************************************************/

void main() {
    // Extract values from G-buffer
    vec3 fragPos    = texture(gPosition, bufferCoords).xyz;
    vec3 fragNormal = texture(gNormal, bufferCoords).xyz;

    fragColor = vec4(0.0, 0.0, 0.0, 1.0);

    // Accumulate lighting from point lights
    for (uint lightIdx = 0U; lightIdx < pointLightsData.length(); lightIdx++) {
        PointLight light    = pointLightsData[lightIdx];
        vec3 lightColor     = light.color.rgb;
        vec3 lightPosition  = light.position.xyz;
        
        float successFraction   = samplePointShadow(fragPos, lightIdx);
        if (successFraction != 0.0) { fragColor.rgb += successFraction * xToonShading(fragPos, fragNormal, lightPosition); }
    }

    // Accumulate lighting from area lights
    for (uint lightIdx = 0U; lightIdx < areaLightsData.length(); lightIdx++) {
        AreaLight light     = areaLightsData[lightIdx];
        vec3 lightColor     = light.color.rgb;
        vec3 lightPosition  = light.position.xyz;

        vec4 fragLightCoord     = light.viewProjection * vec4(fragPos, 1.0);
        float successFraction   = sampleAreaShadow(fragLightCoord, lightIdx);
        if (successFraction != 0.0) { fragColor.rgb += successFraction * xToonShading(fragPos, fragNormal, lightPosition); }
    }
}
