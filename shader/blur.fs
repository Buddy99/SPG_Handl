#version 450

in vec2 TexCoords;

layout (location = 0) uniform sampler2D uFilterMap;
uniform float blurAmount;
out vec4 FragColor;

void main(void)
{
    vec2 sum = vec2(0.0, 0.0);

    vec2 texelSize = 1.0 / vec2(textureSize(uFilterMap, 0));
    for(float x = -blurAmount; x <= blurAmount; x++) 
    {
        for(float y = -blurAmount; y <= blurAmount; y++) 
        {
            vec2 offset = vec2(x, y) * texelSize;

            float depth = texture(uFilterMap, TexCoords + offset).r;

            float dx = dFdx(depth);
            float dy = dFdx(depth);
            float e2 = depth * depth + 0.25 * (dx * dx + dy * dy);

            sum += vec2(depth, e2);
        }
    }
    float numSamples = blurAmount;
    numSamples = numSamples * 2 + 1;
    numSamples *= numSamples;
    sum /= numSamples;

    FragColor = vec4(sum, 0.0, 1.0);
} 