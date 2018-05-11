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

static void printVertices(std::vector<Vertex>& v)
{
  for (int i = 0; i < v.size(); i++)
    std::cout << "[" << v[i].TexCoords.x << "," << v[i].TexCoords.y << "]" << std::endl;
}

static void printIndices(std::vector<unsigned int>& indices)
{
  for (int i = 0; i < indices.size(); i++)
    std::cout << indices[i] << std::endl;
}

class OBJImporter
{
  public:
    void importOBJ(const char* filename, std::vector<Mesh>& meshes)
    {
      std::vector<glm::vec3> temp_vertices;
      std::vector<glm::vec2> temp_uvs;
      std::vector<glm::vec3> temp_normals;
      
      std::vector<Vertex> vertices;
      std::vector<unsigned int> indices;

      std::unordered_map<std::string, unsigned int> verticesMap;

      std::unordered_map<std::string, Material> materialMap;

      std::ifstream in(filename, std::ifstream::in);
      if (!in)
      {
        std::cerr << "Cannot open " << filename << std::endl; exit(1);
      }

      // Strip directory
      std::string filenameS(filename);
      std::string dir = filenameS.substr(0, filenameS.find_last_of("\\/"));

      // First mesh flag
      bool firstMesh = true;

      // Current material
      std::string currentMtl;

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
          std::istringstream s(line.substr(3));
          glm::vec2 v; s >> v.x; s >> v.y;
          temp_uvs.push_back(v);
        }
        else if (line.substr(0,3) == "vn ")
        {
          std::istringstream s(line.substr(3));
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

          vertex1.TexCoords.y = 1 - vertex1.TexCoords.y;
          vertex2.TexCoords.y = 1 - vertex2.TexCoords.y;
          vertex3.TexCoords.y = 1 - vertex3.TexCoords.y;

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
        else if (line.substr(0,7) == "mtllib ")
        {
          std::istringstream s(line.substr(7));
          std::string mtlPath; 
          s >> mtlPath;
          std::cout << mtlPath << std::endl;
          std::string fullPath = dir + "/" + mtlPath;
          importMtl(fullPath.c_str(), materialMap);
        }
        else if (line.substr(0,7) == "usemtl ")
        {
          if (!firstMesh) {
            Mesh mesh(vertices, indices, materialMap[currentMtl]);
            meshes.push_back(mesh);
            vertices.clear();
            indices.clear();
          }

          firstMesh = false;

          std::istringstream s(line.substr(7));
          s >> currentMtl;
        }
        else if (line[0] == '#')
        {
        }
        else
        {
        }
      }

      Mesh mesh(vertices, indices, materialMap[currentMtl]);
      meshes.push_back(mesh);
    }

    void importMtl(const char* filename, std::unordered_map<std::string, Material>& mtlMap)
    {
      std::ifstream in(filename, std::ifstream::in);
      if (!in)
      {
        std::cerr << "Cannot open " << filename << std::endl; exit(1);
      }

      std::string line;
      Material currentMtl = Material();
      bool first = true;
      while (getline(in, line))
      {
        if (line.substr(0,7) == "newmtl ")
        {
          if (!first) {
            mtlMap[currentMtl.name] = currentMtl;
          }

          first = false;

          std::istringstream s(line.substr(7));
          s >> currentMtl.name;
        }
        else if (line.substr(0, 7) == "map_Kd ")
        {
          std::istringstream s(line.substr(7));
          s >> currentMtl.texPath;
        }
        else if (line[0] == '#')
        {
        }
      }

      mtlMap[currentMtl.name] = currentMtl;
    }
};
#endif
