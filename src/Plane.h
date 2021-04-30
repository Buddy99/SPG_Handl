#pragma once

#include "Mesh.h"

class Plane : public Mesh
{
public:
    Plane() : Mesh()
    {
        // positions
        mVertices[0] = glm::vec3(-1.0f, 1.0f, 0.0f);
        mVertices[1] = glm::vec3(-1.0f, -1.0f, 0.0f);
        mVertices[2] = glm::vec3(1.0f, -1.0f, 0.0f);
        mVertices[3] = glm::vec3(1.0f, 1.0f, 0.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;

        // triangle 1
        glm::vec3 edge1 = mVertices[1] - mVertices[0];
        glm::vec3 edge2 = mVertices[2] - mVertices[0];
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);

        // triangle 2
        edge1 = mVertices[2] - mVertices[0];
        edge2 = mVertices[3] - mVertices[0];
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);


        float quadVertices[] = {
            // positions            // normal           // texcoords                      // tangent                          // bitangent
            mVertices[0].x, mVertices[0].y, mVertices[0].z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            mVertices[1].x, mVertices[1].y, mVertices[1].z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            mVertices[2].x, mVertices[2].y, mVertices[2].z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            mVertices[0].x, mVertices[0].y, mVertices[0].z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            mVertices[2].x, mVertices[2].y, mVertices[2].z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            mVertices[3].x, mVertices[3].y, mVertices[3].z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &mVao);
        glGenBuffers(1, &mVbo);
        glBindVertexArray(mVao);
        glBindBuffer(GL_ARRAY_BUFFER, mVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }

    ~Plane()
    {
    }

    void Draw()
    {
        glBindVertexArray(mVao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    // Check if the ray intersects the plane
    bool Intersects(const Ray& ray, float& hitT)
    {
        glm::vec3 pos0 = mModelMatrix * glm::vec4(mVertices[0], 1.0);
        glm::vec3 pos1 = mModelMatrix * glm::vec4(mVertices[1], 1.0);
        glm::vec3 pos2 = mModelMatrix * glm::vec4(mVertices[2], 1.0);

        glm::vec3 dir0 = pos1 - pos0;
        glm::vec3 dir1 = pos2 - pos0;

        glm::vec3 normal = glm::normalize(glm::cross(dir0, dir1));
        glm::vec3 center = pos0 + (dir0 / 2.0f) + (dir1 / 2.0f);

        float denom = glm::dot(normal, ray.mDirection);
        if (abs(denom) > ray.GetEpsilon())
        {
            hitT = glm::dot((center - ray.mOrigin), (normal) / denom);
            if (hitT >= ray.GetEpsilon()) return true;
        }
        return false;
    }

private:
    glm::vec3 mVertices[4];
};