#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int ID;

    Shader()
        : ID(0)
    {
    }

    // Load Shader and compile
    bool load(const char* vertexPath = nullptr, const char* fragmentPath = nullptr, const char* geometryPath = nullptr, const char* tessControlPath = nullptr, const char* tessEvalPath = nullptr, bool link = true)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        std::string tessControlCode;
        std::string tessEvalCode;

        try
        {
            if (vertexPath != nullptr)
            {
                vertexCode = openAndReadFile(vertexPath);
                findAndIncludeFiles(vertexCode);
            }

            if (fragmentPath != nullptr)
            {
                fragmentCode = openAndReadFile(fragmentPath);
                findAndIncludeFiles(fragmentCode);
            }

            if (geometryPath != nullptr)
            {
                geometryCode = openAndReadFile(geometryPath);
                findAndIncludeFiles(geometryCode);
            }


            if (tessControlPath != nullptr)
            {
                tessControlCode = openAndReadFile(tessControlPath);
                findAndIncludeFiles(tessControlCode);
            }

            if (tessEvalPath != nullptr)
            {
                tessEvalCode = openAndReadFile(tessEvalPath);
                findAndIncludeFiles(tessEvalCode);
            }

        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << e.what() << std::endl;
        }
        // 2. compile shaders
        // vertex shader
        unsigned int vertex;
        if (vertexPath != nullptr)
        {
            const char* vShaderCode = vertexCode.c_str();
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX");
        }
        // fragment Shader
        unsigned int fragment;
        if (fragmentPath != nullptr)
        {
            const char* fShaderCode = fragmentCode.c_str();
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, "FRAGMENT");
        }
        // if geometry shader is given, compile geometry shader
        unsigned int geometry;
        if (geometryPath != nullptr)
        {
            const char* gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
        unsigned int tessControl;
        if (tessControlPath != nullptr)
        {
            const char* tcShaderCode = tessControlCode.c_str();
            tessControl = glCreateShader(GL_TESS_CONTROL_SHADER);
            glShaderSource(tessControl, 1, &tcShaderCode, NULL);
            glCompileShader(tessControl);
            checkCompileErrors(tessControl, "TESS_CONTROL");
        }
        unsigned int tessEval;
        if (tessEvalPath != nullptr)
        {
            const char* teShaderCode = tessEvalCode.c_str();
            tessEval = glCreateShader(GL_TESS_EVALUATION_SHADER);
            glShaderSource(tessEval, 1, &teShaderCode, NULL);
            glCompileShader(tessEval);
            checkCompileErrors(tessEval, "TESS_EVAL");
        }
        // shader Program
        ID = glCreateProgram();
        if(vertexPath != nullptr)
            glAttachShader(ID, vertex);
        if(fragmentPath != nullptr)
            glAttachShader(ID, fragment);
        if (geometryPath != nullptr)
            glAttachShader(ID, geometry);
        if (tessControlPath != nullptr)
            glAttachShader(ID, tessControl);
        if (tessEvalPath != nullptr)
            glAttachShader(ID, tessEval);

        if (link)
        {
            glLinkProgram(ID);
            checkCompileErrors(ID, "PROGRAM");
        }
        // delete the shaders as they're linked into our program now and no longer necessery
        if(vertexPath != nullptr)
            glDeleteShader(vertex);

        if(fragmentPath != nullptr)
            glDeleteShader(fragment);

        if (geometryPath != nullptr)
            glDeleteShader(geometry);

        if (tessControlPath != nullptr)
            glDeleteShader(tessControl);

        if (tessEvalPath != nullptr)
            glDeleteShader(tessEval);

        return true;
    }

    bool link()
    {
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        return true;
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use()
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(GetLocation(name), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    {
        glUniform1i(GetLocation(name), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(GetLocation(name), value);
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(GetLocation(name), 1, &value[0]);
    }
    void setVec2(const std::string& name, float x, float y) const
    {
        glUniform2f(GetLocation(name), x, y);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(GetLocation(name), 1, &value[0]);
    }
    void setVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(GetLocation(name), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(GetLocation(name), 1, &value[0]);
    }
    void setVec4(const std::string& name, float x, float y, float z, float w)
    {
        glUniform4f(GetLocation(name), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat) const
    {
        glUniformMatrix2fv(GetLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(GetLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(GetLocation(name), 1, GL_FALSE, &mat[0][0]);
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if(type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if(!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if(!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }

    std::string openAndReadFile(const char* path) const
    {
        std::string code;
        std::ifstream file;

        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            if (path != nullptr)
            {
                file.open(path);
                std::stringstream stream;
                stream << file.rdbuf();
                file.close();
                code = stream.str();
                return code;
            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << e.what() << std::endl;
        }
        return "";
    }

    void findAndIncludeFiles(std::string& code) const
    {
        size_t pos = 0;
        while ((pos = code.find("INCLUDE")) != std::string::npos)
        {
            size_t fileBegin = code.find("\"", pos);
            size_t fileEnd = code.find("\"", fileBegin + 1);

            std::string fileName = code.substr(fileBegin + 1, fileEnd - fileBegin - 1);

            std::string includeCode = openAndReadFile(fileName.c_str());

            code.replace(code.begin() + pos, code.begin() + fileEnd + 1, includeCode);
        }
    }

    GLint GetLocation(const std::string& name) const
    {
        GLint location = glGetUniformLocation(ID, name.c_str());
        //assert(location != -1);
        return location;
    }
};
#endif