#version 330

layout(points) in;
layout(points) out;
layout(max_vertices = 40) out;

// Particle attributes from vertex shader

in vec3 positionPass[];
in vec3 velocityPass[];
in vec3 colorPass[];
in float lifeTimePass[];
in float sizePass[];
in int typePass[];

// Particle attributes out

out vec3 positionOut;
out vec3 velocityOut;
out vec3 colorOut;
out float lifeTimeOut;
out float sizeOut;
out int typeOut;

uniform vec3 position; // Position where new particles are spawned
uniform vec3 gravityVector; // Gravity vector for all particles - updates velocity of particles every frame

// Velocity of new particle - from min to (min+range)
uniform vec3 velocityMin; 
uniform vec3 velocityRange;

uniform vec3 color; // Color of new particle
uniform float size; // Size of new particle

// Lifetime of new particle - from min to (min+range)
uniform float lifeMin;
uniform float lifeRange; 
uniform float timePassed; // Time passed since last frame

uniform vec3 randomSeed; // Seed number for our random number function
vec3 localSeed; // Writable copy of RandomSeed

uniform int numToGenerate; // How many particles will be generated next time, if greater than zero, particles are generated

// Get a random number from zero to one
float randZeroOne()
{
    uint n = floatBitsToUint(localSeed.y * 214013.0 + localSeed.x * 2531011.0 + localSeed.z * 141251.0);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
 
    float res =  2.0 - uintBitsToFloat(n);
    localSeed = vec3(localSeed.x + 147158.0 * res, localSeed.y*res  + 415161.0 * res, localSeed.z + 324154.0*res);
    return res;
}

void main()
{
  localSeed = randomSeed;
  
  // gl_Position doesn't matter, as rendering is discarded

  // Set particle attributes
  positionOut = positionPass[0];
  velocityOut = velocityPass[0];
  if(typePass[0] != 0)positionOut += velocityOut*timePassed;
  if(typePass[0] != 0)velocityOut += gravityVector*timePassed;

  colorOut = colorPass[0];
  lifeTimeOut = lifeTimePass[0]-timePassed;
  sizeOut = sizePass[0];
  typeOut = typePass[0];
    
  if(typeOut == 0) // If the Particle is a particle generator
  {
    // No need to check for lifetime, as the generator is always there
    EmitVertex();
    EndPrimitive();
    
    for(int i = 0; i < numToGenerate; i++) // Spawn as many particles as specified
    {
      positionOut = position;
      velocityOut = velocityMin+vec3(velocityRange.x*randZeroOne(), velocityRange.y*randZeroOne(), velocityRange.z*randZeroOne());
      colorOut = color;
      lifeTimeOut = lifeMin+lifeRange*randZeroOne();
      sizeOut = size;
      typeOut = 1;
      EmitVertex();
      EndPrimitive();
    }
  }
  else if(lifeTimeOut > 0.0) // If it's a normal particle, that still has lifetime remaining
  {
      if(lifeTimeOut < 0.5)
      {
        colorOut.x = 0;
        colorOut.y = 1;
        colorOut.z = 1;
      }
      else if(lifeTimeOut < 1.0)
      {
        colorOut.x = 1;
        colorOut.y = 0;
        colorOut.z = 1;
      }
      // Emit Particle
      EmitVertex();
      EndPrimitive(); 
  }
}