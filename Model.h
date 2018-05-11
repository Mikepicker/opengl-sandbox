#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"
#include "Shader.h"
#include "OBJImporter.h"

#include <vector>

class Model
{
  public:
    Model(const char* filename)
    {
      OBJImporter importer;
      importer.importOBJ(filename, meshes);
    }

    void Draw(Shader& shader)
    {
      for (std::vector<Mesh>::iterator it = meshes.begin(); it != meshes.end(); it++)
      {
        it->Draw(shader);
      }
    }

  private:
    std::vector<Mesh> meshes;
};
#endif
