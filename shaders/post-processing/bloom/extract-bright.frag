#version 460

// HDR framebuffer texture
layout(location = 0) uniform sampler2D hdrBuffer;

// Threshold for deciding bright and dim regions
layout(location = 1) uniform float brightnessThreshold;

// sRGB luminance weights (https://en.wikipedia.org/wiki/Luma_(video))
layout(location = 2) uniform vec3 lumaWeights = vec3(0.2126, 0.7152, 0.0722);

// Quad texture to use with buffer
layout(location = 0) in vec2 bufferCoords;

// Output
layout(location = 0) out vec4 fragColor;

void main() {      
    vec3 hdrColor       = texture(hdrBuffer, bufferCoords).rgb;
    float brightness    = dot(hdrColor, lumaWeights);
    vec3 extractedColor = brightness > brightnessThreshold ? hdrColor : vec3(0.0, 0.0, 0.0);
    fragColor           = vec4(extractedColor, 1.0);
}
