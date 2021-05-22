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

float LinearStep(float low, float high, float value)
{
    return clamp((value - low) / (high - low), 0.0, 1.0);
}

float GetHardShadow(sampler2D shadowMap, vec4 fragPosLightSpace, vec3 fragNormal, vec3 fragPos, vec3 lightPos)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // Hard Shadow
    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

float GetPcfShadow(sampler2D shadowMap, vec4 fragPosLightSpace, vec3 fragNormal, vec3 fragPos, vec3 lightPos, int taps)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -taps; x <= taps; ++x)
    {
        for(int y = -taps; y <= taps; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    float t = taps;
    // Calculate all taps in x
    t = t * 2 + 1;
    // Multiply with y to get all taps
    t *= t;
    shadow /= t;

    return shadow;
}

float chebyshevUpperBound(vec2 moments, float t, float minVariance) 
{   
    float variance = moments.y - (moments.x * moments.x);   
    variance = max(variance, minVariance);    
    float d = t - moments.x;   
    float pMax = variance / (variance + d * d);   
    return max(pMax, float(t < moments.x)); 
} 

float reduceLightBleeding(float light, float amount) 
{     
    return LinearStep(amount, 1.0, light); 
} 

float GetVsmShadow(sampler2D shadowMap, vec4 fragPosLightSpace, float minVariance, float lightBleedOffset)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    vec2 moments = texture(shadowMap, projCoords.xy).rg;
    float p = step(projCoords.z, moments.x);
	float variance = max(moments.y - moments.x * moments.x, minVariance);

	float d = (projCoords.z - moments.x) * 10;
	float pMax = LinearStep(lightBleedOffset, 1.0, variance / (variance + d*d));

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
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - FsIn.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - FsIn.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = GetVsmShadow(uShadowMap, ShadowIn.FragPosLightSpace, 0.2, 0.5);                  
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    

    FragColor = vec4(lighting, 1.0) * uColor;
}