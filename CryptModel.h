#ifndef CRYPT_MODEL_H
#define CRYPT_MODEL_H

#include "Shader.h"
#include "Model.h"
#include "Camera.h"

#include <vector>

struct ShaderParams
{
  float la, ld, ls; // Light Ambient/Diffuse/Specular
  int s;            // Shininess
};

class CryptModel : Model
{
  public:
    CryptModel(const char* filename) : Model(filename)
    {
      basicShader = new Shader("res/shaders/basic/basic.vs", "res/shaders/basic/basic.fs");
      nmShader = new Shader("res/shaders/normalmap/normalmap.vs", "res/shaders/normalmap/normalmap.fs");
    }

    void Draw(glm::mat4& projection, glm::mat4& view, Camera& camera, glm::vec3& lightPos, ShaderParams& basicParams, ShaderParams& normalParams)
    {
      for (std::vector<Mesh>::iterator it = meshes.begin(); it != meshes.end(); it++)
      {
        glm::mat4 model;

        // Draw wall meshes
        if (!it->material.normalPath.empty())
        {
          nmShader->use();
          nmShader->setInt("diffuseMap", 0);
          nmShader->setInt("normalMap", 1);
          nmShader->setMat4("model", model);
          nmShader->setMat4("projection", projection);
          nmShader->setMat4("view", view);
          nmShader->setVec3("viewPos", camera.Position);
          nmShader->setVec3("lightPos", lightPos);
          nmShader->setVec3("light.ambient", glm::vec3(normalParams.la));
          nmShader->setVec3("light.diffuse", glm::vec3(normalParams.ld));
          nmShader->setVec3("light.specular", glm::vec3(normalParams.ls));
          nmShader->setFloat("shininess", normalParams.s);

        } else
        {
          // Draw mesh with diffuse texture
          basicShader->use();
          basicShader->setMat4("model", model);
          basicShader->setMat4("projection", projection);
          basicShader->setMat4("view", view);
          basicShader->setVec3("viewPos", camera.Position);
          basicShader->setFloat("material.shininess", basicParams.s);
          basicShader->setVec3("light.position", lightPos);
          basicShader->setVec3("light.ambient", glm::vec3(basicParams.la));
          basicShader->setVec3("light.diffuse", glm::vec3(basicParams.ld));
          basicShader->setVec3("light.specular", glm::vec3(basicParams.ls));
        }

        
        it->Draw();
      }
    }

  private:
    Shader* nmShader;
    Shader* basicShader;
};
#endif
