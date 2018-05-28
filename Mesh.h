#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include "dep/glm/glm.hpp"
#include "dep/glm/gtc/matrix_transform.hpp"
#include "dep/stb_image/stb_image.h"

#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

static unordered_map<std::string, unsigned int> texturesMap;

struct Vertex {
  // position
  glm::vec3 Position;
  // normal
  glm::vec3 Normal;
  // texCoords
  glm::vec2 TexCoords;
  // tangent
  glm::vec3 Tangent;
  // bitangent
  glm::vec3 Bitangent;
};

struct Texture {
  unsigned int id;
  string type;
  string path;
};

struct Material {
  string name;
  string texPath;
  string normalPath;
  string specularPath;
  string maskPath;
  glm::vec3 ambient = glm::vec3(1.0f, 1.0f, 1.0f);
  glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
  glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
};

// Utility function for loading a 2D texture from file
static unsigned int loadTexture(char const * path)
{
  if (texturesMap.find(std::string(path)) != texturesMap.end())
    return texturesMap[std::string(path)];

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    if (nrComponents == 2)
      format = GL_ALPHA;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  texturesMap[std::string(path)] = textureID;
  return textureID;
}

class Mesh {
  public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    Material material;
    unsigned int diffuseMap, normalMap, maskMap, specularMap;
    unsigned int VAO;
    std::string name;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, Material material)
    {
      this->vertices = vertices;
      this->indices = indices;
      this->material = material;

      /*std::cout << "TEX" << material.texPath << std::endl;
      std::cout << "NRM" << material.normalPath << std::endl;
      std::cout << "MSK" << material.maskPath << std::endl;*/

      if (!material.texPath.empty())
        diffuseMap = loadTexture(material.texPath.c_str());

      if (!material.normalPath.empty())
      {
        normalMap = loadTexture(material.normalPath.c_str());
        computeTangents();
      }

      if (!material.specularPath.empty())
        specularMap = loadTexture(material.specularPath.c_str());

      if (!material.maskPath.empty())
        maskMap = loadTexture(material.maskPath.c_str());

      // now that we have all the required data, set the vertex buffers and its attribute pointers.
      setupMesh();
    }

    // Render the mesh
    void Draw(const Shader& shader)
    {
      // Bind textures
      if (!material.texPath.empty())
      {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
      }

      shader.setBool("hasNormalMap", false);
      if (!material.normalPath.empty())
      {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        shader.setBool("hasNormalMap", true);
      }

      shader.setBool("hasSpecularMap", false);
      if (!material.maskPath.empty())
      {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        shader.setBool("hasSpecularMap", true);
      }

      shader.setBool("hasMaskMap", false);
      if (!material.maskPath.empty())
      {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, maskMap);
        shader.setBool("hasMaskMap", true);
      }

      // Set material params
      shader.setVec3("material.ambient", material.ambient);
      shader.setVec3("material.diffuse", material.diffuse);
      shader.setVec3("material.specular", material.specular);

      // Draw mesh
      glBindVertexArray(VAO);
      glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
    }

private:
    /*  Render data  */
    unsigned int VBO, EBO;

    /*  Functions    */
    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
    }

    void computeTangents()
    {
      for (unsigned int i = 0; i < indices.size(); i += 3)
      {
        Vertex v1, v2, v3;
        v1 = vertices[indices[i]];
        v2 = vertices[indices[i + 1]];
        v3 = vertices[indices[i + 2]];

        /*glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);  
        glm::vec2 uv4(1.0f, 1.0f);*/
        glm::vec2 uv1 = v1.TexCoords;
        glm::vec2 uv2 = v2.TexCoords;
        glm::vec2 uv3 = v3.TexCoords;

        glm::vec3 tangent;

        glm::vec3 edge1 = v2.Position - v1.Position;
        glm::vec3 edge2 = v3.Position - v1.Position;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);

        v1.Tangent = tangent;
        v2.Tangent = tangent;
        v3.Tangent = tangent;

        vertices[indices[i]] = v1;
        vertices[indices[i + 1]] = v2;
        vertices[indices[i + 2]] = v3;
      }

      // Normalize
      for (unsigned int i = 0; i < indices.size(); i++)
      {
        vertices[indices[i]].Tangent = glm::normalize(vertices[indices[i]].Tangent);
      }
    }
};
#endif
