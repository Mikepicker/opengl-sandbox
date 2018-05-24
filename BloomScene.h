#ifndef BLOOM_SCENE_H
#define BLOOM_SCENE_H

#include "Scene.h"
#include "Model.h"

class BloomScene : public Scene
{
  public:
    BloomScene(GLFWwindow* window, unsigned int width, unsigned int height)
      : Scene(window, width, height)
    {
      monkey = new Model("res/models/monkey.obj"); 

      m_LightPos = glm::vec3(0.0f, 0.0f, 4.0f);

      // Configure (floating point) framebuffers
      glGenFramebuffers(1, &hdrFBO);
      glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

      // Create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
      glGenTextures(2, colorBuffers);
      for (unsigned int i = 0; i < 2; i++)
      {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // Attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
      }

      // Create and attach depth buffer (renderbuffer)
      unsigned int rboDepth;
      glGenRenderbuffers(1, &rboDepth);
      glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
      // Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
      glDrawBuffers(2, attachments);
      // Finally check if framebuffer is complete
      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      // Ping-pong-framebuffer for blurring
      glGenFramebuffers(2, pingpongFBO);
      glGenTextures(2, pingpongColorbuffers);
      for (unsigned int i = 0; i < 2; i++)
      {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
          std::cout << "Framebuffer not complete!" << std::endl;
      }

      // Lighting info
      // -------------
      // positions
      lightPositions.push_back(glm::vec3( 0.0f, 0.5f,  1.5f));
      lightPositions.push_back(glm::vec3(-4.0f, 0.5f, -3.0f));
      lightPositions.push_back(glm::vec3( 3.0f, 0.5f,  1.0f));
      lightPositions.push_back(glm::vec3(-.8f,  2.4f, -1.0f));
      // colors
      lightColors.push_back(glm::vec3(2.0f, 2.0f, 2.0f));
      lightColors.push_back(glm::vec3(1.5f, 0.0f, 0.0f));
      lightColors.push_back(glm::vec3(0.0f, 0.0f, 1.5f));
      lightColors.push_back(glm::vec3(0.0f, 1.5f, 0.0f));

      shader = new Shader("res/shaders/bloom/bloom.vs", "res/shaders/bloom/bloom.fs");
      shaderLight = new Shader("res/shaders/bloom/bloom.vs", "res/shaders/bloom/light.fs");
      shaderBlur = new Shader("res/shaders/bloom/blur.vs", "res/shaders/bloom/blur.fs");
      shaderBloomFinal = new Shader("res/shaders/bloom/bloom_final.vs", "res/shaders/bloom/bloom_final.fs");

      shader->use();
      shader->setInt("diffuseTexture", 0);
      shaderBlur->use();
      shaderBlur->setInt("image", 0);
      shaderBloomFinal->use();
      shaderBloomFinal->setInt("scene", 0);
      shaderBloomFinal->setInt("bloomBlur", 1);
    }

    void Draw()
    {
      Scene::Draw();
     
      // 1. render scene into floating point framebuffer
      // -----------------------------------------------
      glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)s_WindowWidth / (float)s_WindowHeight, 0.1f, 100.0f);
      glm::mat4 view = camera.GetViewMatrix();
      glm::mat4 model;
      shader->use();
      shader->setMat4("projection", projection);
      shader->setMat4("view", view);

      // set lighting uniforms
      for (unsigned int i = 0; i < lightPositions.size(); i++)
      {
        shader->setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
        shader->setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
      }
      shader->setVec3("viewPos", camera.Position);

      model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
      shader->setMat4("model", model);
      monkey->Draw(*shader);

      //DrawLamp();

      // finally show all the light sources as bright cubes
      shaderLight->use();
      shaderLight->setMat4("projection", projection);
      shaderLight->setMat4("view", view);

      for (unsigned int i = 0; i < lightPositions.size(); i++)
      {
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(lightPositions[i]));
        model = glm::scale(model, glm::vec3(0.25f));
        shaderLight->setMat4("model", model);
        shaderLight->setVec3("lightColor", lightColors[i]);
        m_Lamp->Draw(*shaderLight);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      // 2. blur bright fragments with two-pass Gaussian Blur
      // --------------------------------------------------
      bool horizontal = true, first_iteration = true;
      unsigned int amount = 10;
      shaderBlur->use();
      for (unsigned int i = 0; i < amount; i++)
      {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        shaderBlur->setInt("horizontal", horizontal);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
        renderQuad();
        horizontal = !horizontal;
        if (first_iteration)
          first_iteration = false;
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      shaderBloomFinal->use();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
      shaderBloomFinal->setInt("bloom", bloom);
      shaderBloomFinal->setFloat("exposure", exposure);
      renderQuad();

      std::cout << "bloom: " << (bloom ? "on" : "off") << "| exposure: " << exposure << std::endl;
    }

  private:
    Shader* shader;
    Shader* shaderLight;
    Shader* shaderBlur;
    Shader* shaderBloomFinal;

    unsigned int hdrFBO;
    unsigned int colorBuffers[2];
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    unsigned int quadVAO, quadVBO;

    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;

    float exposure = 1.0f;
    bool bloom = true;

    Model* monkey;

    void renderQuad()
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
        // setup plane VAO
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
