#ifndef GODRAYS_H
#define GODRAYS_H

#include "Shader.h"

#include <glad/glad.h>
#include <iostream>

#include "dep/glm/glm.hpp"
#include "dep/glm/gtc/matrix_transform.hpp"
#include "dep/glm/gtc/type_ptr.hpp"

struct GodraysParams
{
  float exposure;
  float decay;
  float density;
  float weight;
};

class Godrays
{
  public:
    Godrays(unsigned int windowWidth, unsigned int windowHeight) : windowWidth(windowWidth), windowHeight(windowHeight)
    {
      // Shader
      godraysShader = new Shader("res/shaders/godrays/godrays.vs", "res/shaders/godrays/godrays.fs");

      // Generate FBO
      glGenFramebuffers(1, &framebuffer);
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

      // Generate texture
      glGenTextures(1, &texColorBuffer);
      glBindTexture(GL_TEXTURE_2D, texColorBuffer);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth/4, windowHeight/4, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glBindTexture(GL_TEXTURE_2D, 0);

      // attach it to currently bound framebuffer object
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

      // Generate and bind render buffer for depth and stencil tests
      glGenRenderbuffers(1, &rbo);
      glBindRenderbuffer(GL_RENDERBUFFER, rbo);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      // Attach rbo to fbo
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

      // Check attachments and unbind fbo
      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      // Generate texture quad
      float quadVertices[] = {
        -1.0f, 1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f,  0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f,  1.0f, 1.0f
      };
      glGenVertexArrays(1, &quadVAO);
      glGenBuffers(1, &quadVBO);
      glBindVertexArray(quadVAO);
      glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    void Bind()
    {
      glViewport(0, 0, windowWidth/4, windowHeight/4);
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Unbind()
    {
      glViewport(0, 0, windowWidth, windowHeight);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 lightPos, GodraysParams& params)
    {
      glm::vec4 clipLight = projection * view * glm::vec4(lightPos.x, lightPos.y, lightPos.z, 1.0);
      glm::vec3 ndcLight = clipLight / clipLight.w;

      godraysShader->use();

      godraysShader->setVec2("lightPositionOnScreen", glm::vec2((ndcLight.x + 1) / 2, (ndcLight.y + 1) / 2));
      godraysShader->setInt("firstPass", 0);
      godraysShader->setFloat("exposure", params.exposure);
      godraysShader->setFloat("decay", params.decay);
      godraysShader->setFloat("density", params.density);
      godraysShader->setFloat("weight", params.weight);
      
      glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

      glBindVertexArray(quadVAO);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texColorBuffer);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      glDisable(GL_BLEND);
    }
  
  private:
    unsigned int framebuffer, texColorBuffer, rbo;
    unsigned int quadVAO, quadVBO;
    unsigned int windowWidth, windowHeight;
    Shader* godraysShader;
};

#endif
