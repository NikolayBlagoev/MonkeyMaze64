#version 460

#define PI 3.1415926535897932384626433832795

struct PointLight {
    vec4 position;
    vec4 color;
};

struct AreaLight {
    vec4 position;
    vec4 color;
    mat4 viewProjection;
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

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a         = roughness * roughness;
    float a2        = a * a;
    float NdotH     = max(dot(N, H), 0.0);
    float NdotH2    = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom       = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 computeReflectance(vec3 fragPos, vec3 fragNormal,
                        vec3 albedo, float metallic, float roughness,
                        vec3 lightPos, vec3 lightColor,
                        vec3 view, vec3 F0) {
    // Radiance
    vec3 L              = normalize(lightPos - fragPos);
    vec3 H              = normalize(view + L);
    float lightFragDist = length(lightPos - fragPos);
    float attenuation   = 1.0 / (lightFragDist * lightFragDist);
    vec3 radiance       = lightColor * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(fragNormal, H, roughness);   
    float G   = GeometrySmith(fragNormal, view, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, view), 0.0), F0);
        
    vec3 nom        = NDF * G * F; 
    float denom     = 4.0 * max(dot(fragNormal, view), 0.0) * max(dot(fragNormal, L), 0.0) + 0.0001; // + 0.0001 to prevent division by zero
    vec3 specular   = nom / denom;
    
    // kS is equal to Fresnel
    vec3 kS = F;

    // For energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should be equal to 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;

    // Multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	  

    // Scale light by N.L
    float NdotL = max(dot(fragNormal, L), 0.0);        

    // Add to outgoing radiance Lo
    return (kD * albedo / PI + specular) * radiance * NdotL;  // Note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
}

/*****************************************************************************************************/

void main() {
    // Extract values from G-buffer
    vec3 fragPos        = texture(gPosition, bufferCoords).xyz;
    vec3 fragNormal     = texture(gNormal, bufferCoords).xyz;
    vec3 fragAlbedo     = texture(gAlbedo, bufferCoords).rgb;
    vec3 fragMaterial   = texture(gMaterial, bufferCoords).rgb;
    float metallic      = fragMaterial.r;
    float roughness     = fragMaterial.g;
    float ao            = fragMaterial.b;

    vec3 positionToCamera = normalize(cameraPos - fragPos);
    fragColor = vec4(0.0, 0.0, 0.0, 1.0);

    // Calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, fragAlbedo, metallic);

    // Accumulate lighting from point lights
    for (uint lightIdx = 0U; lightIdx < pointLightsData.length(); lightIdx++) {
        PointLight light    = pointLightsData[lightIdx];
        vec3 lightColor     = light.color.rgb;
        vec3 lightPosition  = light.position.xyz;
        
        float successFraction   = samplePointShadow(fragPos, lightIdx);
        if (successFraction != 0.0) { fragColor.rgb += computeReflectance(fragPos, fragNormal, fragAlbedo, metallic, roughness, lightPosition, lightColor, positionToCamera, F0); }
    }

    // Accumulate lighting from area lights
    for (uint lightIdx = 0U; lightIdx < areaLightsData.length(); lightIdx++) {
        AreaLight light     = areaLightsData[lightIdx];
        vec3 lightColor     = light.color.rgb;
        vec3 lightPosition  = light.position.xyz;

        vec4 fragLightCoord     = light.viewProjection * vec4(fragPos, 1.0);
        float successFraction   = sampleAreaShadow(fragLightCoord, lightIdx);
        if (successFraction != 0.0) { fragColor.rgb += computeReflectance(fragPos, fragNormal, fragAlbedo, metallic, roughness, lightPosition, lightColor, positionToCamera, F0); }
    }
    
    // Ambient lighting
    fragColor.rgb += vec3(0.03) * fragAlbedo * ao;
}
