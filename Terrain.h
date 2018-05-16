#ifndef TERRAIN_H
#define TERRAIN_H

#include "Mesh.h"

#include <vector>

class Terrain
{
  public:
    Terrain(const char* heightmapPath, const char* grassPath, const char* snowPath, const char* dirtPath)
    {
      heightmap = loadTexture(heightmapPath);
      grass = loadTexture(grassPath);
      snow = loadTexture(snowPath);
      dirt = loadTexture(dirtPath);
      setupTerrain(heightmapPath);
      setupMesh();
    }

    void Draw()
    {
      // Bind textures
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, heightmap);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, grass);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, snow);
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, dirt);

      // Draw mesh
      glBindVertexArray(VAO);
      glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
      glBindVertexArray(0); 
    }

  private:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    Material material;

    unsigned int VAO, VBO, EBO;
    unsigned int heightmap, grass, snow, dirt;

    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
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

    void setupTerrain(const char* heightmapPath)
    {
      // Heightmap data
      int width, height, nrComponents;
      unsigned char *data = stbi_load(heightmapPath, &width, &height, &nrComponents, 0);

      vertices = std::vector<Vertex>(width * height);
      for (unsigned int z = 0; z < height; z++)
      {
        for (unsigned int x = 0; x < width; x++)
        {
          Vertex v;
          v.Normal = glm::vec3(0);
          v.Position = glm::vec3((float)x/width, 0, (float)z/height);

          /*v.Position.x -= w/2;
          v.Position.z -= h/2;*/
          
          vertices[z*width + x] = v;
        }
      }

      // Indices
      unsigned int c = 1;
      for (unsigned int i = 0; i < width * (height-1); i++)
      {
        indices.push_back(i);
        indices.push_back(i + width);

        //std::cout << "[" << i << ", " << i+width;
        if (c % width == 0 && c < width * (height-1))
        {
          indices.push_back(i + width);
          indices.push_back(i + 1);
          //std::cout << ", " << i+width << ", " << i+1;
        }

        c++;

        //std::cout << "]" << std::endl;
      }
    }
};
#endif
