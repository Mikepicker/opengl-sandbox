#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"
#include "OBJImporter.h"

#include <vector>

struct ShaderParams
{
  float la, ld, ls; // Light Ambient/Diffuse/Specular
  int s;            // Shininess
};

class Model
{
  public:
    Model(const char* filename)
    {
      OBJImporter importer;
      importer.importOBJ(filename, meshes);
    }

    virtual void Draw()
    {
      for (std::vector<Mesh>::iterator it = meshes.begin(); it != meshes.end(); it++)
      {
        it->Draw();
      }
    }

  protected:
    std::vector<Mesh> meshes;
};
#endif
