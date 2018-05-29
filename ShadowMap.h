#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

#include "Shader.h"
#include "Model.h"

#include "dep/glm/glm.hpp"
#include "dep/glm/gtc/matrix_transform.hpp"
#include "dep/glm/gtc/type_ptr.hpp"

#include <glad/glad.h>

const unsigned int SHADOW_WIDTH = 800, SHADOW_HEIGHT = 800;

class ShadowMap
{
  public:
    
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 100.0f;

    ShadowMap(unsigned int windowWidth, unsigned int windowHeight)
      : windowWidth(windowWidth), windowHeight(windowHeight)
    {
      // Init shaders
      shadowDepthShader = new Shader("res/shaders/shadow/omni.vs", "res/shaders/shadow/omni.fs", "res/shaders/shadow/omni.gs");
      debugShadowMapShader = new Shader("res/shaders/shadow/debug.vs", "res/shaders/shadow/debug.fs");

      // configure depth map FBO
      // -----------------------
      glGenFramebuffers(1, &depthMapFBO);
      // create depth cubemap texture
      glGenTextures(1, &depthCubemap);
      glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
      for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

      // attach depth texture as FBO's depth buffer
      glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);

      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void ComputeShadowMap(Model& model, glm::mat4 modelMatrix, glm::vec3 lightPos)
    {
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // 0. create depth cubemap transformation matrices
      // -----------------------------------------------
      glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
      std::vector<glm::mat4> shadowTransforms;
      shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

      // 1. render scene to depth cubemap
      // --------------------------------
      glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
      glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
      glClear(GL_DEPTH_BUFFER_BIT);
      shadowDepthShader->use();
      for (unsigned int i = 0; i < 6; ++i)
        shadowDepthShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
      shadowDepthShader->setMat4("model", modelMatrix);
      shadowDepthShader->setFloat("far_plane", far_plane);
      shadowDepthShader->setVec3("lightPos", lightPos);
      model.Draw(*shadowDepthShader);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glViewport(0, 0, windowWidth, windowHeight);
    }

    void Bind()
    {
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    }

  private:
    unsigned int windowWidth, windowHeight;

    Shader* shadowDepthShader;
    Shader* debugShadowMapShader;

    unsigned int depthMapFBO;
    unsigned int depthCubemap;
    unsigned int quadVAO = 0;
    unsigned int quadVBO;

};
#endif
