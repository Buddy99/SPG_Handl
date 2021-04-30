#version 430

layout (binding = 0) uniform sampler2D gSampler;

in vec2 texCoord;
in vec4 colorPart;

out vec4 fragColor;

void main()
{
  // Texture the particles
  vec4 texColor = texture2D(gSampler, texCoord);
  fragColor = texColor * colorPart;
}