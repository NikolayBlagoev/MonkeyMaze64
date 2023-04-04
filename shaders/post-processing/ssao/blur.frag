#version 460

layout(location = 0) uniform sampler2D ssaoRenderTex;

// User-defined parameter(s) to control effect
layout(location = 1) uniform int kernelLength;

layout(location = 0) in vec2 texCoords;

layout(location = 0) out float fragColor;

void main()  {
    vec2 texelSize      = 1.0 / vec2(textureSize(ssaoRenderTex, 0));
    int offsetMagnitude = kernelLength / 2;
    float result        = 0.0;

    // Loop over fragments in kernel window and use random values to sample neighbours for blur
    for (int x = -offsetMagnitude; x < offsetMagnitude; x++) {
        for (int y = -offsetMagnitude; y < offsetMagnitude; y++)  {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result      += texture(ssaoRenderTex, texCoords + offset).r;
        }
    }

    fragColor = result / float(kernelLength * kernelLength);
}  