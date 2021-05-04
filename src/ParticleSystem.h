#pragma once

#include <glm/glm.hpp>
#include <glBasics/shader.h>
#include <glBasics/texture2D.h>

#include <string>

class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();

	bool InitParticleSystem();

	void UpdateParticles(float timeStep);
	void RenderParticles();

	void SetGeneratorProperties(const glm::vec3& position, const glm::vec3& velocityMin, const glm::vec3& velocityMax, const glm::vec3& gravity, const glm::vec3 color, float minLifeTime, float maxLifeTime, float size, float every, int numToGenerate);
	void SetGeneratorPosition(const glm::vec3& position);
	int GetNumParticles() const;

	void SetMatrices(const glm::mat4& projection, const glm::mat4& viewMat, const glm::vec3& view, const glm::vec3& upVector);

	Texture2D* mTexture;
	float mUpdateRate;
private:
	void CheckInit() const;

	static const int sBufferSize = 2;
	static const int sMaxParticles = 100000;

	bool mInitalized;

	unsigned int mTransformFeedbackBuffer;

	unsigned int mVbos[sBufferSize];
	unsigned int mVaos[sBufferSize];

	unsigned int mQuery;

	int mCurrentReadBuffer;
	int mNumParticles;

	glm::mat4 mProjection;
	glm::mat4 mView;
	glm::vec3 mQuad1;
	glm::vec3 mQuad2;

	float mElapsedTime;

	glm::vec3 mPosition;
	glm::vec3 mVelocityMin;
	glm::vec3 mVelocityRange;
	glm::vec3 mGravity;
	glm::vec3 mColor;

	float mLifeTimeMin;
	float mLifeTimeRange;
	float mSize;

	int mNumToGenerate;

	Shader* renderShader;
	Shader* updateShader;
};