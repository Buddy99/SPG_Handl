#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Shader.h"

class TextRenderer
{
public:
	struct Character
	{
		unsigned int TexId;
		glm::ivec2 Size;
		glm::ivec2 Bearing;
		unsigned int Advance;
	};

	TextRenderer()
		: mCharacters()
		, mShader()
		, mTextVAO(0)
		, mTextVBO(0)
	{
	}

	~TextRenderer()
	{
        if (mTextVAO != 0)
        {
            glDeleteVertexArrays(1, &mTextVAO);
        }
        if (mTextVBO != 0)
        {
            glDeleteBuffers(1, &mTextVBO);
        }
	}

    void SetupArrays()
    {
        // Configure Text VAO & VBO
        glGenVertexArrays(1, &mTextVAO);
        glGenBuffers(1, &mTextVBO);
        glBindVertexArray(mTextVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mTextVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

	void LoadShader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
	{
		mShader.load(vertexPath, fragmentPath, geometryPath);
	}

    void SetProjection(glm::mat4 projection, const std::string& paramName = "uProjection")
    {
        mShader.use();
        mShader.setMat4(paramName, projection);
    }

    void RenderFormattedText(const std::string& text, float x, float y, float scale, glm::vec3 color, float spacing)
    {
        std::istringstream textStream(text);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(textStream, part))
        {
            parts.push_back(part);
        }
        for (auto& string : parts)
        {
            for (size_t pos = 0; pos < string.length(); pos++)
            {
                if (string[pos] == '\t')
                {
                    switch (pos % 4)
                    {
                        default:
                        case 0:
                            string.replace(pos, 1, "    ");
                            pos += 3;
                            break;

                        case 1:
                            string.replace(pos, 1, "   ");
                            pos += 2;
                            break;

                        case 2:
                            string.replace(pos, 1, "  ");
                            pos += 1;
                            break;

                        case 3:
                            string.replace(pos, 1, " ");
                            break;
                    }
                }
            }
        }
        RenderText(parts[0], x, y, scale, color);
        float currentSpacing = 0;
        float currentScale = mCharacters['M'].Size.y * scale;
        for (size_t i = 1; i < parts.size(); i++)
        {   
            currentSpacing += spacing + currentScale;
            RenderText(parts[i], x, y - currentSpacing - i * currentScale, scale, color);
        }
    }

	void RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color)
	{
        // activate corresponding render state	
        mShader.use();
        mShader.setVec3("uTextColor", color);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(mTextVAO);

        // iterate through all characters
        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++)
        {
            Character ch = mCharacters[*c];

            float xpos = x + ch.Bearing.x * scale;
            float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;
            // update VBO for each character
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };
            // render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.TexId);
            // update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, mTextVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
	}

	bool SetupFreetype(const std::string& absFontPath)
	{
        SetupArrays();
        FT_Library ft;
        // Init -> 0 no error
        if (FT_Init_FreeType(&ft))
        {
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return false;
        }
        // Load Font
        if (absFontPath.empty())
        {
            std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
            return false;
        }

        FT_Face face;
        if (FT_New_Face(ft, absFontPath.c_str(), 0, &face))
        {
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
            return false;
        }
        else
        {
            // set size to load glyphs as
            FT_Set_Pixel_Sizes(face, 0, 48);

            // disable byte-alignment restriction
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            mShader.use();
            // load first 128 characters of ASCII set
            for (unsigned char c = 0; c < 128; c++)
            {
                // Load character glyph 
                if (FT_Load_Char(face, c, FT_LOAD_RENDER))
                {
                    std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                    continue;
                }
                // generate texture
                unsigned int texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RED,
                    face->glyph->bitmap.width,
                    face->glyph->bitmap.rows,
                    0,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    face->glyph->bitmap.buffer
                );
                // set texture options
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                // now store character for later use
                Character character = {
                    texture,
                    glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    static_cast<unsigned int>(face->glyph->advance.x)
                };
                mCharacters.insert(std::pair<char, Character>(c, character));
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        // destroy FreeType once we're finished
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        return true;
	}

protected:
	std::map<GLchar, Character> mCharacters;
	Shader mShader;
	unsigned int mTextVAO;
	unsigned int mTextVBO;
};