#version 450

in SHADOW_OUT
{
    vec4 FragPosLightSpace;
} ShadowIn;

uniform float shadowPass;

float HandleShadowPass()
{
    if(shadowPass == 1.0)
    {
        return 1.0;
    }
    else
    {
        return 0.0;
    }
}

// Reduce Light Bleeding
float LinearStep(float low, float high, float value)
{
    // Interpolate & Clamp
    return clamp((value - low) / (high - low), 0.0, 1.0);
}

// Sample variance shadow map (shadow map stores distance z to the light)
float GetVsmShadow(sampler2D shadowMap, vec4 fragPosLightSpace, float minVariance, float lightBleedOffset)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Calculate variance from Shadowmap
    vec2 moments = texture(shadowMap, projCoords.xy).rg;
    float p = step(projCoords.z, moments.x);
	float variance = max(moments.y - moments.x * moments.x, minVariance);

    // Calculate probability of being lit (using Chebyshev's inequality)
	float d = (projCoords.z - moments.x) * 10;  // Calculate distance from the mean
	float pMax = LinearStep(lightBleedOffset, 1.0, variance / (variance + d*d)); // Calculate upper bound

	return 1 - min(max(p, pMax), 1.0);
} 

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
} FsIn;

layout (binding = 0) uniform sampler2D uDiffuseMap;
layout (binding = 1) uniform sampler2D uShadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec4 uColor;

out vec4 FragColor;

void main()
{     
    if(HandleShadowPass() == 1.0)
    {
        return;
    }
    vec3 color = texture(uDiffuseMap, FsIn.TexCoords).rgb;
    vec3 normal = normalize(FsIn.Normal);
    vec3 lightColor = vec3(0.3);
    // Ambient
    vec3 ambient = 0.3 * color;
    // Diffuse
    vec3 lightDir = normalize(lightPos - FsIn.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // Specular
    vec3 viewDir = normalize(viewPos - FsIn.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // Calculate shadow (VSM)
    float shadow = GetVsmShadow(uShadowMap, ShadowIn.FragPosLightSpace, 0.2, 0.5);                  
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    

    FragColor = vec4(lighting, 1.0) * uColor;
}