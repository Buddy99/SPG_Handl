#pragma once

#include <glad/glad.h>
#include <vector>
#include "shader.h"
#include "texture.h"

class Material
{
public:
	Material()
		: mShader(0)
		, mTextures()
		, mSharedShader(false)
	{
	}

	~Material()
	{
		for (auto tex : mTextures)
		{
			if (!tex->mShared)
			{
				delete tex;
			}
		}
		mTextures.clear();
		if (!mSharedShader)
		{
			delete mShader;
		}
	}

	inline void use()
	{
		mShader->use();
		for (auto tex : mTextures)
		{
			tex->use();
		}
	}

	void UseSharedShader(Shader* shader)
	{
		mSharedShader = true;
		mShader = shader;
	}

	void UseShader(Shader* shader)
	{
		mSharedShader = false;
		mShader = shader;
	}

	Shader* GetShader()
	{
		return mShader;
	}

	std::vector<Texture*> mTextures;

private:
	Shader* mShader;
	bool mSharedShader;
};