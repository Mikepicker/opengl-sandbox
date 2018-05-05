#ifndef OBJ_IMPORTER_H
#define OBJ_IMPORTER_H

#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <sstream>

#include "dep/glm/glm.hpp"
#include "dep/glm/gtc/matrix_transform.hpp"
#include "dep/glm/gtc/type_ptr.hpp"

#include "Mesh.h"

static void printVector(std::vector<glm::vec3>& v)
{
  for (int i = 0; i < v.size(); i++)
    std::cout << "[" << v[i].x << "," << v[i].y << "," << v[i].z << "]" << std::endl;
}

static void printIndices(std::vector<unsigned int>& indices)
{
  for (int i = 0; i < indices.size(); i++)
    std::cout << indices[i] << std::endl;
}

class OBJImporter
{
  public:
    void importOBJ(const char* filename, std::vector<Vertex> &vertices, std::vector<unsigned int> &indices)
    {
      std::vector<glm::vec3> temp_vertices;
      std::vector<glm::vec2> temp_uvs;
      std::vector<glm::vec3> temp_normals;

      std::unordered_map<std::string, unsigned int> verticesMap;

      std::ifstream in(filename, std::ifstream::in);
      if (!in)
      {
        std::cerr << "Cannot open " << filename << std::endl; exit(1);
      }

      std::string line;
      while (getline(in, line))
      {
        if (line.substr(0,2) == "v ")
        {
          std::istringstream s(line.substr(2));
          glm::vec3 v; s >> v.x; s >> v.y; s >> v.z;
          temp_vertices.push_back(v);
        }
        else if (line.substr(0,3) == "vt ")
        {
          std::istringstream s(line.substr(2));
          glm::vec3 v; s >> v.x; s >> v.y; s >> v.z;
          temp_uvs.push_back(v);
        }
        else if (line.substr(0,3) == "vn ")
        {
          std::istringstream s(line.substr(2));
          glm::vec3 v; s >> v.x; s >> v.y; s >> v.z;
          temp_normals.push_back(v);
        }
        else if (line.substr(0,2) == "f ")
        {
          unsigned int v1, v2, v3;
          unsigned int vt1, vt2, vt3;
          unsigned int vn1, vn2, vn3;

          bool hasUV = true;

          // v1/v2/v3
          int matches = sscanf(line.c_str(), "f %u/%u/%u %u/%u/%u %u/%u/%u", &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);
          if (matches != 9)
          {
            // v1//vn1
            matches = sscanf(line.c_str(), "f %u//%u %u//%u %u//%u", &v1, &vn1, &v2, &vn2, &v3, &vn3);
            if (matches != 6) {
              std::cout << "Unsupported file!\n" << std::endl;
              return;
            }

            hasUV = false;
          }

          v1--; v2--; v3--;
          vt1--; vt2--; vt3--;
          vn1--; vn2--; vn3--;

          Vertex vertex1; 
          vertex1.Position = temp_vertices[v1];
          vertex1.TexCoords = hasUV ? temp_uvs[vt1] : glm::vec3(0.0f);
          vertex1.Normal = temp_normals[vn1];

          Vertex vertex2; 
          vertex2.Position = temp_vertices[v2];
          vertex2.TexCoords = hasUV ? temp_uvs[vt2] : glm::vec3(0.0f);
          vertex2.Normal = temp_normals[vn2];

          Vertex vertex3; 
          vertex3.Position = temp_vertices[v3];
          vertex3.TexCoords = hasUV ? temp_uvs[vt3] : glm::vec3(0.0f);
          vertex3.Normal = temp_normals[vn3];

          std::string k1 = std::to_string(v1) + "/" + std::to_string(vt1) + "/" + std::to_string(vn1); 
          std::string k2 = std::to_string(v2) + "/" + std::to_string(vt2) + "/" + std::to_string(vn2); 
          std::string k3 = std::to_string(v3) + "/" + std::to_string(vt3) + "/" + std::to_string(vn3); 

          if (verticesMap.find(k1) == verticesMap.end())
          {
            vertices.push_back(vertex1);
            verticesMap[k1] = vertices.size()-1; 
          }
          if (verticesMap.find(k2) == verticesMap.end())
          {
            vertices.push_back(vertex2);
            verticesMap[k2] = vertices.size()-1; 
          }
          if (verticesMap.find(k3) == verticesMap.end())
          {
            vertices.push_back(vertex3);
            verticesMap[k3] = vertices.size()-1; 
          }

          indices.push_back(verticesMap[k1]);
          indices.push_back(verticesMap[k2]);
          indices.push_back(verticesMap[k3]);
        }
        else if (line[0] == '#')
        {
        }
        else
        {
        }
      }
    }
};
#endif
