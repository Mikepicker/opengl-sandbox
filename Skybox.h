#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <iostream>

#include <glad/glad.h>

#include "dep/glm/glm.hpp"
#include "dep/glm/gtc/matrix_transform.hpp"
#include "dep/glm/gtc/type_ptr.hpp"
#include "dep/stb_image/stb_image.h"

#include "Shader.h"

class Skybox
{
  public:
    Skybox()
    {
      float skyboxVertices[] = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
      };

      // Create cubemap
      std::vector<std::string> faces =
      {
        "res/skyboxes/hw_sahara/sahara_ft.tga",
        "res/skyboxes/hw_sahara/sahara_bk.tga",
        "res/skyboxes/hw_sahara/sahara_up.tga",
        "res/skyboxes/hw_sahara/sahara_dn.tga",
        "res/skyboxes/hw_sahara/sahara_rt.tga",
        "res/skyboxes/hw_sahara/sahara_lf.tga",
      };
      cubeTex = loadCubemap(faces);  

      // Skybox VAO
      glGenVertexArrays(1, &skyboxVAO);
      glGenBuffers(1, &skyboxVBO);
      glBindVertexArray(skyboxVAO);
      glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

      // Create shader
      skyboxShader = new Shader("res/shaders/cubemap/cubemap.vs", "res/shaders/cubemap/cubemap.fs");

    }

    void Draw(glm::mat4& projection, glm::mat4& view) {
      glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
      skyboxShader->use();
      glm::mat3 viewNoTrans = glm::mat4(glm::mat3(view)); // remove translation from the view matrix
      skyboxShader->setMat4("view", viewNoTrans);
      skyboxShader->setMat4("projection", projection);
      // Skybox cube
      glBindVertexArray(skyboxVAO);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      glBindVertexArray(0);
      glDepthFunc(GL_LESS); // set depth function back to default

    }

  private:
    unsigned int skyboxVBO;
    unsigned int skyboxVAO;

    Shader* skyboxShader;

    // Cubemap
    unsigned int cubeTex;

    unsigned int loadCubemap(std::vector<std::string> faces)
    {
      unsigned int textureID;
      glGenTextures(1, &textureID);
      glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

      int width, height, nrChannels;
      for (unsigned int i = 0; i < faces.size(); i++)
      {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
          glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
              0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
              );
          stbi_image_free(data);
        }
        else
        {
          std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
          stbi_image_free(data);
        }
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

      return textureID;
    }
};
#endif
