#version 330

// Particle attributes in
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 velocity;
layout (location = 2) in vec3 color;
layout (location = 3) in float lifeTime;
layout (location = 4) in float size;
layout (location = 5) in int type;

// Pass particle attributes to GS
out vec3 positionPass;
out vec3 velocityPass;
out vec3 colorPass;
out float lifeTimePass;
out float sizePass;
out int typePass;

void main()
{
  positionPass = position;
  velocityPass = velocity;
  colorPass = color;
  lifeTimePass = lifeTime;
  sizePass = size;
  typePass = type;
}