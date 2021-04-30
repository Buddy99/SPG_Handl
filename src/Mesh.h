#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Ray.h"

class Mesh
{
public:
	Mesh()
		: mVao(0)
		, mVbo(0)
		, mModelMatrix()
	{
	}

	~Mesh()
	{
		if (mVao != 0)
		{
			glDeleteVertexArrays(1, &mVao);
			mVao = 0;
		}
		if (mVbo != 0)
		{
			glDeleteBuffers(1, &mVbo);
			mVbo = 0;
		}
	}

	void Draw()
	{
	}

	bool Intersects(const Ray& ray, float& hitT)
	{
		return true;
	}

	unsigned int mVao;
	unsigned int mVbo;

	glm::mat4 mModelMatrix;
};