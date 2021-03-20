#version 430

// The vertexshader is called as often as the texture is large (in this case 64x64x64 times)

layout (binding = 0) uniform sampler3D tex; //  3D Texture

out vec3 varTexture;
out int mcCase;   // Marching Cubes Case (Index)

const float step = 1.0 / 63.0;  // Depends on the texturesize

// Vertexshader generates only points (no triangles)
void main(void)
{
    // ID (containing all 3 coordinates) is divided in 3 coordinates (x, y and z)
    // Bit-Shifting
    int id = gl_VertexID;
    int x = id & 0x3F;
    int y = (id >> 6) & 0x3F;
    int z = (id >> 12) & 0x3F;

    vec3 xyz = vec3(x, y, z);
    gl_Position = vec4(xyz, 1.0);
    // Multiply with step to get values between 0 and 1
    varTexture = xyz * step;

    // Sample the 3D Texture to get the density values at the 8 corners
    int d1 = int(texture(tex, varTexture).r < 0.5f);
    int d2 = int(texture(tex, varTexture + vec3(step, 0.0, 0.0)).r < 0.5f);
    int d3 = int(texture(tex, varTexture + vec3(step, 0.0, step)).r < 0.5f);
    int d4 = int(texture(tex, varTexture + vec3(0.0, 0.0, step)).r < 0.5f);
    int d5 = int(texture(tex, varTexture + vec3(0.0, step, 0.0)).r < 0.5f);
    int d6 = int(texture(tex, varTexture + vec3(step, step, 0.0)).r < 0.5f);
    int d7 = int(texture(tex, varTexture + vec3(step, step, step)).r < 0.5f);
    int d8 = int(texture(tex, varTexture + vec3(0.0, step, step)).r < 0.5f);
    // Determine which of the 256 marching cube cases we have for this cell
    mcCase = (d1 << 7) | (d2 << 6) | (d3 << 5) | (d4 << 4) | 
                (d5 << 3) | (d6 << 2) | (d7 << 1) | d8;
}