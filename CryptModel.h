#ifndef CRYPT_MODEL_H
#define CRYPT_MODEL_H

#include "Shader.h"
#include "Model.h"
#include "Camera.h"

#include <vector>

class CryptModel : public Model
{
  public:
    CryptModel(const char* filename) : Model(filename)
    {
      basicShader = new Shader("res/shaders/basic/basic.vs", "res/shaders/basic/basic.fs");
      nmShader = new Shader("res/shaders/normalmap/normalmap.vs", "res/shaders/normalmap/normalmap.fs");
    }

    void DrawCrypt(glm::mat4& projection, glm::mat4& view, Camera& camera, glm::vec3& lightPos, Shader& uberShader)
    {
      for (std::vector<Mesh>::iterator it = meshes.begin(); it != meshes.end(); it++)
      {
        glm::mat4 model;

        // Draw wall meshes
        uberShader.setBool("hasNormalMap", !it->material.normalPath.empty());
        
        it->Draw(uberShader);
      }
    }

  private:
    Shader* nmShader;
    Shader* basicShader;
};
#endif
