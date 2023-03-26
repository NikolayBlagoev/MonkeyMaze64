#version 460

// HDR framebuffer texture
layout(location = 0) uniform sampler2D hdrBuffer;

// HDR rendering params
layout(location = 1) uniform bool hdr;
layout(location = 2) uniform float exposure;
layout(location = 3) uniform float gamma;

// Post-processing results
layout(location = 4) uniform sampler2D bloomFilter;

// Quad texture to use with HDR buffer
layout(location = 0) in vec2 bufferCoords;

// Output
layout(location = 0) out vec4 fragColor;

void main() {
    // Combine post-processing results
    vec3 hdrColor   = texture(hdrBuffer, bufferCoords).rgb;
    hdrColor       += texture(bloomFilter, bufferCoords).rgb;
    
    if (hdr) {
        // vec3 result = hdrColor / (hdrColor + vec3(1.0));     // Reinhard tone mapping
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);    // Exposure tone mapping
        result      = pow(result, vec3(1.0 / gamma));           // Gamma correction
        fragColor   = vec4(result, 1.0);
    }
    else {
        vec3 result = pow(hdrColor, vec3(1.0 / gamma));
        fragColor   = vec4(result, 1.0);
    }
}
