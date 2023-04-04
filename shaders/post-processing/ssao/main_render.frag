#version 460

// Sample vectors SSBO
layout(binding = 0) buffer samples { vec3 samplesData[]; };

// G-buffer position and normal; Random rotation texture
layout(location = 0) uniform sampler2D gPosition;
layout(location = 1) uniform sampler2D gNormal;
layout(location = 2) uniform sampler2D noiseTex;

// Screen viewProjection matrix
layout(location = 3) uniform mat4 viewProjection;

// Render resolution
layout(location = 4) uniform vec2 renderResolution;

// User-defined parameter(s) to control effect
layout(location = 5) uniform float kernelLength;
layout(location = 6) uniform float radius;
layout(location = 7) uniform float bias;
layout(location = 8) uniform float power;

layout(location = 0) in vec2 texCoords;

layout(location = 0) out float fragColor;

void main() {
    // Tile noise texture over screen based on render resolution divided by noise size
    const vec2 noiseScale = vec2(renderResolution.x / kernelLength, renderResolution.y / kernelLength); 

    // Get input from textures
    vec3 fragPos    = texture(gPosition, texCoords).xyz;
    vec3 normal     = normalize(texture(gNormal, texCoords).rgb);
    vec3 randomVec  = normalize(texture(noiseTex, texCoords * noiseScale).xyz);

    // Create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent    = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent  = cross(normal, tangent);
    mat3 TBN        = mat3(tangent, bitangent, normal);

    // Iterate over all samples and calculate occlusion factor
    float occlusion = 0.0;
    for (uint sampleIdx = 0U; sampleIdx < samplesData.length(); sampleIdx++) {
        // Get sample position
        vec3 samplePos  = TBN * samplesData[sampleIdx]; // From tangent to view-space
        samplePos       = fragPos + samplePos * radius; 
        
        // Project sample position to get position on screen/G-buffer
        vec4 offset = vec4(samplePos, 1.0);
        offset      = viewProjection * offset;  // From world to screen space
        offset.xyz  /= offset.w;                // Convert from homogeneous coordinates
        offset.xyz  = offset.xyz * 0.5 + 0.5;   // Transform to range [0.0, 1.0]
        
        // Get depth corresponding to sample
        float sampleDepth = texture(gPosition, offset.xy).z;
        
        // Range check & accumulate
        float rangeCheck    = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion           += (sampleDepth > samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }

    // Normalise and raise to arbitrary power for effect strengthening
    occlusion = 1.0 - (occlusion / samplesData.length());
    fragColor = pow(occlusion, power);
}