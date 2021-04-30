#version 430

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

layout (binding = 0) uniform sampler2D diffuseMap;
layout (binding = 1) uniform sampler2D normalMap;
layout (binding = 2) uniform sampler2D depthMap;

uniform float heightScale;

uniform float steps;
uniform float refinementSteps;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{  
    // Calculate the size of each layer (depending on the specified steps)
    float layerDepth = 1.0 / steps;
    // Depth of current layer
    float currentLayerDepth = 0.0;
    // The amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale; 
    vec2 deltaTexCoords = P / steps;
  
    // Get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
    
    // Steps
    while(currentLayerDepth < currentDepthMapValue)
    {
        // Shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // Get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;  
        // Get depth of next layer
        currentLayerDepth += layerDepth;  
    }

    // Calculate the size of each refined layer (depending on the specified refinement steps)
    layerDepth /= refinementSteps;
    deltaTexCoords /= refinementSteps;

    // Refinement Steps
    while(currentLayerDepth > currentDepthMapValue)
    {
        // Shift texture coordinates along direction of P
        currentTexCoords += deltaTexCoords;
        // Get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;  
        // Get depth of next layer
        currentLayerDepth -= layerDepth;  
    }
    
    return currentTexCoords;
}

void main()
{           
    // Offset texture coordinates with Displacement Mapping
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = fs_in.TexCoords;
    
    texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);       
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
    {
        discard;
    }
        
    // Obtain normals from normal map
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);   
   
    // Get diffuse color
    vec3 color = texture(diffuseMap, texCoords).rgb;
    // Ambient
    vec3 ambient = 0.1 * color;
    // Diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // Specular    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}