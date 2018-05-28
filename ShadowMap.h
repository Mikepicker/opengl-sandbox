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

    ShadowMap(unsigned int windowWidth, unsigned int windowHeight)
      : windowWidth(windowWidth), windowHeight(windowHeight)
    {
      // Init shaders
      shadowDepthShader = new Shader("res/shaders/shadow/shadowdepth.vs", "res/shaders/shadow/shadowdepth.fs");
      debugShadowMapShader = new Shader("res/shaders/shadow/debug.vs", "res/shaders/shadow/debug.fs");

      // Depth map FBO
      glGenFramebuffers(1, &depthMapFBO);

      // Create depth texture
      glGenTextures(1, &depthMap);
      glBindTexture(GL_TEXTURE_2D, depthMap);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
      glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

      // Attach depth texture as FBO's depth buffer
      glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void ComputeShadowMap(glm::vec3& lightPos, Model& model, glm::mat4 modelMatrix)
    {
      // Render depth of scene to texture (from light's perspective)
      lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
      lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
      lightSpaceMatrix = lightProjection * lightView;

      // 1. Render scene from light's point of view
      shadowDepthShader->use();
      shadowDepthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
      shadowDepthShader->setMat4("model", modelMatrix);

      glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
      glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        //glCullFace(GL_FRONT); // Avoid Peter-Panning
        model.Draw(*shadowDepthShader);
        //glCullFace(GL_BACK);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      glViewport(0, 0, windowWidth, windowHeight);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Bind()
    {
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, depthMap);
    }

    void DrawDebug()
    {
      // Render Depth map to quad for visual debugging
      debugShadowMapShader->use();
      debugShadowMapShader->setInt("depthMap", 0);
      debugShadowMapShader->setFloat("near_plane", near_plane);
      debugShadowMapShader->setFloat("far_plane", far_plane);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, depthMap);
      DrawShadowQuad();
    }

  private:
    unsigned int windowWidth, windowHeight;

    Shader* shadowDepthShader;
    Shader* debugShadowMapShader;

    unsigned int depthMapFBO;
    unsigned int depthMap;
    unsigned int quadVAO = 0;
    unsigned int quadVBO;

    float near_plane = 1.0f, far_plane = 100.5f;

    // Draw shadow fbo quad
    void DrawShadowQuad()
    {
      if (quadVAO == 0)
      {
          float quadVertices[] = {
              // positions        // texture Coords
              -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
              -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
               1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
               1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
          };

          // Setup plane VAO
          glGenVertexArrays(1, &quadVAO);
          glGenBuffers(1, &quadVBO);
          glBindVertexArray(quadVAO);
          glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
          glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
          glEnableVertexAttribArray(0);
          glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
          glEnableVertexAttribArray(1);
          glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
      }
      glBindVertexArray(quadVAO);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      glBindVertexArray(0);
    }
};
#endif
