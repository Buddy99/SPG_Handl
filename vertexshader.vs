#version 430

// The vertexshader is called as often as the texture is large (in this case 64x64x64 times)

layout (binding = 0) uniform sampler3D tex; //  3D Texture

// 3D Texture size
uniform int texWidth;
uniform int texHeight;
uniform int texDepth;

out vec3 varTexture;
out int mcCase;   // Marching Cubes Case (Index)

// Vertexshader generates only points (no triangles)
void main(void)
{
    // ID (containing all 3 coordinates) is divided in 3 coordinates (x, y and z)
    int id = gl_VertexID;
    int x = id % texWidth;
    int y = (id / texWidth) % texHeight;
    int z = (id / (texWidth * texHeight)) % texDepth;

    float widthStep = 1.0f / (texWidth - 1);
    float heightStep = 1.0f / (texHeight - 1);
    float depthStep = 1.0f / (texDepth - 1);

    vec3 xyz = vec3(x, y, z);
    gl_Position = vec4(xyz, 1.0);
    // Multiply with steps to get values between 0 and 1
    varTexture = vec3(x * widthStep, y * heightStep, z * depthStep);

    // Sample the 3D Texture to get the density values at the 8 corners
    int d1 = int(texture(tex, varTexture).r < 0.5f);
    int d2 = int(texture(tex, varTexture + vec3(widthStep, 0.0, 0.0)).r < 0.5f);
    int d3 = int(texture(tex, varTexture + vec3(widthStep, 0.0, depthStep)).r < 0.5f);
    int d4 = int(texture(tex, varTexture + vec3(0.0, 0.0, depthStep)).r < 0.5f);
    int d5 = int(texture(tex, varTexture + vec3(0.0, heightStep, 0.0)).r < 0.5f);
    int d6 = int(texture(tex, varTexture + vec3(widthStep, heightStep, 0.0)).r < 0.5f);
    int d7 = int(texture(tex, varTexture + vec3(widthStep, heightStep, depthStep)).r < 0.5f);
    int d8 = int(texture(tex, varTexture + vec3(0.0, heightStep, depthStep)).r < 0.5f);
    // Determine which of the 256 marching cube cases we have for this cell
    mcCase = (d1 << 7) | (d2 << 6) | (d3 << 5) | (d4 << 4) | 
                (d5 << 3) | (d6 << 2) | (d7 << 1) | d8;
}