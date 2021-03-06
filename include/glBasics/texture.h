#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

class Texture{
public:
    Texture(unsigned int tex = 0, unsigned int texLocation = GL_TEXTURE0)
        : mTex(tex)
        , mTexLocation(texLocation)
        , mShared(false)
    {
    }

    ~Texture()
    {
    }

    inline void use()
    {
        glActiveTexture(mTexLocation);
        glBindTexture(GL_TEXTURE_2D, mTex);
    }

    unsigned int mTex;
    unsigned int mTexLocation;
    bool mShared;
};

// TEXTURE_H
#endif