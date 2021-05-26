#include "ParticleSystem.h"
#include "Particle.h"
#include <glm/gtc/matrix_transform.hpp>
#include <assert.h>

ParticleSystem::ParticleSystem()
	: mInitalized(false)
	, mCurrentReadBuffer(0)
	, mTransformFeedbackBuffer(0)
	, mQuery(0)
	, mTexture(nullptr)
{
	mVbos[0] = 0;
	mVaos[0] = 0;
}

ParticleSystem::~ParticleSystem()
{
	if (mTransformFeedbackBuffer != 0)
	{
		glDeleteTransformFeedbacks(1, &mTransformFeedbackBuffer);
	}
	if (mQuery != 0)
	{
		glDeleteQueries(1, &mQuery);
	}
	if (mVbos[0] != 0)
	{
		glDeleteBuffers(sBufferSize, mVbos);
	}
	if (mVaos[0] != 0)
	{
		glDeleteVertexArrays(sBufferSize, mVaos);
	}
	delete mTexture;
	delete updateShader;
	delete renderShader;
}

bool ParticleSystem::InitParticleSystem()
{
	assert(!mInitalized && "Particle system already initalized!");

	// All particle attributes
	const char* sVaryings[] = {
		"positionOut",
		"velocityOut",
		"colorOut",
		"lifeTimeOut",
		"sizeOut",
		"typeOut"
	};

	unsigned int varyingSize = sizeof(sVaryings) / sizeof(sVaryings[0]);

	// Load shader
	updateShader = new Shader();
	updateShader->load("shader/updateParticle.vs", nullptr, "shader/updateParticle.gs", nullptr, nullptr, false);
	renderShader = new Shader();
	renderShader->load("shader/renderParticle.vs", "shader/renderParticle.fs", "shader/renderParticle.gs");

	// Tell OpenGL, which vertex attributes should be recorded by transform feedback
	for (int i = 0; i < varyingSize; i++)
	{
		glTransformFeedbackVaryings(updateShader->ID, varyingSize, sVaryings, GL_INTERLEAVED_ATTRIBS);
	}
	updateShader->link();

	// Create all necessary buffers
	glGenTransformFeedbacks(1, &mTransformFeedbackBuffer);
	glGenQueries(1, &mQuery);

	// Generate two buffers for storing particles (double buffering)
	glGenBuffers(sBufferSize, mVbos);	// Buffersize = 2
	glGenVertexArrays(sBufferSize, mVaos);

	// Initialize the generator particle
	Particle initParticle;
	initParticle.Type = ParticleType::GENERATOR;

	for (int i = 0; i < sBufferSize; i++)
	{
		glBindVertexArray(mVaos[i]);
		glBindBuffer(GL_ARRAY_BUFFER, mVbos[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * sMaxParticles, nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle), &initParticle);

		for (unsigned int j = 0; j < varyingSize; j++)
		{
			glEnableVertexAttribArray(j);
		}

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);	// Position
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)12);	// Velocity
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)24);	// Color
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)36);	// Lifetime
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)40);	// Size
		glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)44);	// Type
	}
	mCurrentReadBuffer = 0;
	mNumParticles = 1;

	mInitalized = true;
	return true;
}

float grandf(float fMin, float fAdd)
{
	float fRandom = float(rand() % (RAND_MAX + 1)) / float(RAND_MAX);
	return fMin + fAdd * fRandom;
}

void ParticleSystem::UpdateParticles(float timeStep)
{
	CheckInit();

	// Update Particles
	updateShader->use();
	updateShader->setVec3("position", mPosition);
	updateShader->setVec3("gravityVector", mGravity);
	updateShader->setVec3("velocityMin", mVelocityMin);
	updateShader->setVec3("velocityRange", mVelocityRange);
	updateShader->setVec3("color", mColor);
	updateShader->setFloat("size", mSize);
	updateShader->setFloat("lifeMin", mLifeTimeMin);
	updateShader->setFloat("lifeRange", mLifeTimeRange);
	updateShader->setFloat("timePassed", timeStep);

	mElapsedTime += timeStep;

	if (mElapsedTime > mUpdateRate)
	{
		// Produce a new particle generation
		updateShader->setInt("numToGenerate", mNumToGenerate);
		mElapsedTime -= mUpdateRate;

		glm::vec3 randomSeed = glm::vec3(grandf(-10.0f, 20.0f), grandf(-10.0f, 20.0f), grandf(-10.0f, 20.0f));
		updateShader->setVec3("randomSeed", randomSeed);
	}
	else
	{
		updateShader->setInt("numToGenerate", 0);
	}

	// Disable rasterization (since no graphical output is desired yet)
	glEnable(GL_RASTERIZER_DISCARD);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbackBuffer);

	glBindVertexArray(mVaos[mCurrentReadBuffer]);
	// Re-enable velocity (which is not used in the render Shader, only in the update Shader)
	glEnableVertexAttribArray(1);

	// Where to store the results of transform feedback
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mVbos[1 - mCurrentReadBuffer]);

	// Count the number of outputted primitives
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, mQuery);
	// Start recording of outputted geometry
	glBeginTransformFeedback(GL_POINTS);

	// Draw
	glDrawArrays(GL_POINTS, 0, mNumParticles);

	// End recording of outputted geometry
	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glGetQueryObjectiv(mQuery, GL_QUERY_RESULT, &mNumParticles);

	// Swap Read- and Write-Buffer
	mCurrentReadBuffer = 1 - mCurrentReadBuffer;

	// Unbind any transform feedbacks
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	// Re-enable rasterization
	glDisable(GL_RASTERIZER_DISCARD);
}

void ParticleSystem::RenderParticles()
{
	CheckInit();

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	// Render Particles
	renderShader->use();
	mTexture->use();
	renderShader->setMat4("projection", mProjection);
	renderShader->setMat4("view", mView);
	renderShader->setVec3("quad1", mQuad1);
	renderShader->setVec3("quad2", mQuad2);

	glBindVertexArray(mVaos[mCurrentReadBuffer]);
	// Disable velocity, since it's not needed for rendering
	glDisableVertexAttribArray(1);

	// Draw
	glDrawArrays(GL_POINTS, 0, mNumParticles);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void ParticleSystem::SetGeneratorProperties(const glm::vec3& position, const glm::vec3& velocityMin, const glm::vec3& velocityMax, const glm::vec3& gravity, const glm::vec3 color, float minLifeTime, float maxLifeTime, float size, float every, int numToGenerate)
{
	mPosition = position;						// Where are the particles generated
	mVelocityMin = velocityMin;					// Minimum velocity
	mVelocityRange = velocityMax - velocityMin;	// Maximum velocity
	mGravity = gravity;							// Gravity force applied to particles
	mColor = color;								// Color

	mSize = size;								// Rendered Size
	mLifeTimeMin = minLifeTime;					// Minimum lifetime in seconds
	mLifeTimeRange = maxLifeTime - minLifeTime;	// Maximum lifetime in seconds

	mUpdateRate = every;				// When should the next particlle generation be spawned
	mElapsedTime = 0.8f;						// Set Elapsed time so that some particles are spawned instantly 

	mNumToGenerate = numToGenerate;				// How many particles should be spawned
}

void ParticleSystem::SetGeneratorPosition(const glm::vec3& position)
{
	mPosition = position;
}

int ParticleSystem::GetNumParticles() const
{
	return mNumParticles;
}

void ParticleSystem::SetMatrices(const glm::mat4& projection, const glm::mat4& viewMat, const glm::vec3& view, const glm::vec3& upVector)
{
	// Set matrices and quad for billboarding
	mProjection = projection;
	mView = viewMat;

	// Calculate the billboarded plane vectors
	mQuad1 = glm::cross(view, upVector);
	mQuad1 = glm::normalize(mQuad1);
	mQuad2 = glm::cross(view, mQuad1);
	mQuad2 = glm::normalize(mQuad2);
}

void ParticleSystem::CheckInit() const
{
	assert(mInitalized && "Particle system not initalized before use");
}