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
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    std::unordered_map<std::string, unsigned int> verticesMap;

    std::unordered_map<std::string, Material> materialMap;

    void importOBJ(const char* filename, std::vector<Mesh>& meshes)
    {
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

      // Current object
      std::string currentObj;

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
          unsigned int v1, v2, v3, v4;
          unsigned int vt1, vt2, vt3, vt4;
          unsigned int vn1, vn2, vn3, vn4;

          bool hasUV = true;
          bool done = false;

          // v1/v2/v3/v4
          int matches = sscanf(line.c_str(), "f %u/%u/%u %u/%u/%u %u/%u/%u %u/%u/%u", &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3, &v4, &vt4, &vn4);
          if (matches == 12)
          {
            done = true;
            pushVertex(v1, vt1, vn1, hasUV);
            pushVertex(v2, vt2, vn2, hasUV);
            pushVertex(v3, vt3, vn3, hasUV);
            pushVertex(v1, vt1, vn1, hasUV);
            pushVertex(v3, vt3, vn3, hasUV);
            pushVertex(v4, vt4, vn4, hasUV);
          }

          // v1/v2/v3
          matches = sscanf(line.c_str(), "f %u/%u/%u %u/%u/%u %u/%u/%u", &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);
          if (matches == 9 && !done)
          {
            done = true;
            pushVertex(v1, vt1, vn1, hasUV);
            pushVertex(v2, vt2, vn2, hasUV);
            pushVertex(v3, vt3, vn3, hasUV);
          }

          // v1//vn1
          matches = sscanf(line.c_str(), "f %u//%u %u//%u %u//%u", &v1, &vn1, &v2, &vn2, &v3, &vn3);
          if (matches == 6 && !done)
          {
            done = true;
            hasUV = false;
            pushVertex(v1, vt1, vn1, hasUV);
            pushVertex(v2, vt2, vn2, hasUV);
            pushVertex(v3, vt3, vn3, hasUV);
          }

          if (!done)
          {
            std::cout << "Unsupported file!\n" << std::endl;
            return;
          }
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
            mesh.name = currentObj;
            meshes.push_back(mesh);
            vertices.clear();
            indices.clear();
          }

          firstMesh = false;

          std::istringstream s(line.substr(7));
          s >> currentMtl;
        }
        else if (line.substr(0,2) == "o " || line.substr(0,2) == "g ")
        {
          //currentMtl = "";
          std::istringstream s(line.substr(2));
          s >> currentObj;
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
            currentMtl = Material();
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
        else if (line.substr(0, 7) == "map_Ks ")
        {
          std::istringstream s(line.substr(7));
          s >> currentMtl.specularPath;
        }
        else if (line.substr(0, 9) == "map_bump ")
        {
          std::istringstream s(line.substr(9));
          s >> currentMtl.normalPath;
        }
        else if (line.substr(0, 6) == "map_d ")
        {
          std::istringstream s(line.substr(6));
          s >> currentMtl.maskPath;
        }
        else if (line.substr(0, 3) == "Ka ")
        {
          float r, g, b;
          int matches = sscanf(line.c_str(), "Ka %f/%f/%f", &r, &g, &b);
          if (matches == 3)
            currentMtl.ambient = glm::vec3(r, g, b);
        }
        else if (line.substr(0, 3) == "Kd ")
        {
          float r, g, b;
          int matches = sscanf(line.c_str(), "Kd %f/%f/%f", &r, &g, &b);
          if (matches == 3)
            currentMtl.diffuse = glm::vec3(r, g, b);
        }
        else if (line.substr(0, 3) == "Ks ")
        {
          float r, g, b;
          int matches = sscanf(line.c_str(), "Ks %f/%f/%f", &r, &g, &b);
          if (matches == 3)
            currentMtl.specular = glm::vec3(r, g, b);
        }
        else if (line[0] == '#')
        {
        }
      }

      mtlMap[currentMtl.name] = currentMtl;
    }

    void pushVertex(unsigned int v, unsigned int vt, unsigned int vn, bool hasUV)
    {
      v--;
      vt--;
      vn--;

      Vertex vertex; 
      vertex.Position = temp_vertices[v];
      vertex.TexCoords = hasUV ? temp_uvs[vt] : glm::vec3(0.0f);
      vertex.Normal = temp_normals[vn];

      vertex.TexCoords.y = 1 - vertex.TexCoords.y;

      std::string k = std::to_string(v) + "/" + std::to_string(vt) + "/" + std::to_string(vn); 

      if (verticesMap.find(k) == verticesMap.end())
      {
        vertices.push_back(vertex);
        verticesMap[k] = vertices.size()-1; 
      }

      indices.push_back(verticesMap[k]);
    }
};
#endif
