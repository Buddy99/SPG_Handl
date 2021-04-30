#version 430

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
} fs_in;

layout (binding = 0) uniform sampler2D uDiffuseMap;

out vec4 FragColor;

void main()
{     
    // Set color      
    FragColor = texture(uDiffuseMap, fs_in.TexCoords);
}