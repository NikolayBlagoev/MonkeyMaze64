#version 460

// Base image to blur
layout(location = 0) uniform sampler2D image;

// Whether we are computing the horizontal or vertical part of the separable convolution
layout(location = 1) uniform bool horizontal;

// Precomputed weight from Gaussian (uniform) distribution
layout(location = 2) uniform float weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

layout(location = 0) in vec2 bufferCoords;

layout(location = 0) out vec4 fragColor;

void main()
{             
    vec2 texOffset  = 1.0 / textureSize(image, 0);                      // Size of single texel
    vec3 result     = texture(image, bufferCoords).rgb * weights[0];    // Current fragment's contribution
    if (horizontal) {
        for (int iteration = 1; iteration < 5; iteration++) {
            result += texture(image, bufferCoords + vec2(texOffset.x * iteration, 0.0)).rgb * weights[iteration];
            result += texture(image, bufferCoords - vec2(texOffset.x * iteration, 0.0)).rgb * weights[iteration];
        }
    }
    else {
        for (int iteration = 1; iteration < 5; iteration++) {
            result += texture(image, bufferCoords + vec2(0.0, texOffset.y * iteration)).rgb * weights[iteration];
            result += texture(image, bufferCoords - vec2(0.0, texOffset.y * iteration)).rgb * weights[iteration];
        }
    }
    fragColor = vec4(result, 1.0);
}
