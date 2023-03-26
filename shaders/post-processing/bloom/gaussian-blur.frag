#version 460

// Base image to blur
layout(location = 0) uniform sampler2D image;

// Whether we are computing the horizontal or vertical part of the separable convolution
layout(location = 1) uniform bool horizontal;

// Sampling parameters
uniform float offsets[3] = float[](0.0, 1.3846153846, 3.2307692308);           // Offsets to sample the texture such that bilinear sampling gets the correct amount of contribution from neighbouring texels
uniform float weights[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);  // Precomputed weights from Gaussian (uniform) distribution

layout(location = 0) in vec2 bufferCoords;

layout(location = 0) out vec4 fragColor;

// More efficient Gaussian sampling relying on bilinear sampling (https://www.rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/)
void main() {             
    vec2 texOffset  = 1.0 / textureSize(image, 0);                      // Size of single texel
    vec3 result     = texture(image, bufferCoords).rgb * weights[0];    // Current fragment's contribution
    if (horizontal) {
        for (int iteration = 1; iteration < 3; iteration++) {
            result += texture(image, bufferCoords + vec2(texOffset.x * offsets[iteration], 0.0)).rgb * weights[iteration];
            result += texture(image, bufferCoords - vec2(texOffset.x * offsets[iteration], 0.0)).rgb * weights[iteration];
        }
    }
    else {
        for (int iteration = 1; iteration < 3; iteration++) {
            result += texture(image, bufferCoords + vec2(0.0, texOffset.y * offsets[iteration])).rgb * weights[iteration];
            result += texture(image, bufferCoords - vec2(0.0, texOffset.y * offsets[iteration])).rgb * weights[iteration];
        }
    }
    fragColor = vec4(result, 1.0);
}
