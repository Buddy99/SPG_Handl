#version 430

layout (binding = 0) uniform sampler3D tex;

in vec3 varTextureG;
in vec3 varTexture;

out vec4 fragColor;

void main(void)
{
    // float f = texture(tex, varTextureG).r;

    // Set color according to Vertexposition (makes everything colourful)
    fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}