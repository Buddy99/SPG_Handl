#version 450

out SHADOW_OUT
{
    vec4 FragPosLightSpace;
} ShadowOut;

uniform mat4 lightSpaceMatrix;
uniform float shadowPass;

float HandleShadowPass(mat4 model, vec3 pos)
{
    if(shadowPass == 1.0)
    {
        // Set Position
        gl_Position = lightSpaceMatrix * model * vec4(pos, 1.0);
        return 1.0;
    }
    else
    {
        ShadowOut.FragPosLightSpace = lightSpaceMatrix * model * vec4(pos, 1.0);
        return 0.0;
    }
}

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
} VsOut;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    if(HandleShadowPass(model, aPos) == 1.0)
    {
        return;
    }

    VsOut.FragPos = vec3(model * vec4(aPos, 1.0));   
    VsOut.Normal = transpose(inverse(mat3(model))) * aNormal;
    VsOut.TexCoords = aTexCoords;   

    // Set position
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}