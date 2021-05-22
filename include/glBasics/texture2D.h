#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H

#include <glad/glad.h>

#include "Texture.h"

class Texture2D : public Texture
{
public:
    Texture2D(unsigned int tex = 0, unsigned int texLocation = GL_TEXTURE0)
        :Texture(tex, texLocation)
    {
    }

    ~Texture2D()
    {
    }

    void use(unsigned int overrideLocation = GL_TEXTURE) override
    {
        if (overrideLocation == GL_TEXTURE)
        {
            glActiveTexture(mTexLocation);
        }
        else
        {
            glActiveTexture(overrideLocation);
        }
        glBindTexture(GL_TEXTURE_2D, mTex);
    }
};

// TEXTURE_2D_H
#endif