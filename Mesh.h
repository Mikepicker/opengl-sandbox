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

using namespace std;

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
};

class Mesh {
  public:
  /*  Mesh Data  */
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    Material material;
    unsigned int diffuseMap;
    unsigned int VAO;

    /*  Functions  */
    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, Material material)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->material = material;

        if (!material.texPath.empty()) {
          diffuseMap = loadTexture(material.texPath.c_str());
        }

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // Render the mesh
    void Draw(Shader& shader)
    {
      // Bind textures
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, diffuseMap);
      
      // Draw mesh
      glBindVertexArray(VAO);
      glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);

    }

    // Utility function for loading a 2D texture from file
    unsigned int loadTexture(char const * path)
    {
      unsigned int textureID;
      glGenTextures(1, &textureID);

      int width, height, nrComponents;
      unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
      if (data)
      {
        GLenum format;
        if (nrComponents == 1)
          format = GL_RED;
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

      return textureID;
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
};
#endif
