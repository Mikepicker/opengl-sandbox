#ifndef FBO_SCENE_H
#define FBO_SCENE_H

#include "Scene.h"
#include "Shader.h"
#include "Model.h"
#include "OBJImporter.h"

#include <iostream>
#include <vector>

class FBOScene : public Scene
{
  public:
    FBOScene(GLFWwindow* window, unsigned int width, unsigned int height)
      : Scene(window, width, height)
    {
      // Light position
      lightPos = glm::vec3(1.2f, 1.0f, 2.0f);

      // configure global opengl state
      glEnable(GL_DEPTH_TEST);

      // build and compile our shader zprogram
      modelShader = new Shader("res/shaders/basic/basic.vs", "res/shaders/basic/basic.fs");
      lampShader = new Shader("res/shaders/basic/lamp.vs", "res/shaders/basic/lamp.fs");
      screenShader = new Shader("screen.vs", "screen.fs");

      // Load models
      cube = new Model("res/models/crate.obj");
      lamp = new Model("res/models/cube.obj");

      // Generate and bind to FBO
      glGenFramebuffers(1, &framebuffer);
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

      // generate texture
      glGenTextures(1, &texColorBuffer);
      glBindTexture(GL_TEXTURE_2D, texColorBuffer);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
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

      // Create quad VAO
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

    void Draw()
    {
      // per-frame time logic
      // --------------------
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      // input
      // -----
      processInput();

      // render
      // ------
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


      // view/projection transformations
      projection = glm::perspective(glm::radians(camera.Zoom), (float)s_WindowWidth / (float)s_WindowHeight, 0.1f, 100.0f);
      view = camera.GetViewMatrix();

      // First pass - bind fbo
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
      glEnable(GL_DEPTH_TEST);

      // Draw model
      DrawModel(); 

      // Draw lamp
      DrawLamp();

      // Second pass - unbind fbo
      glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      glDisable(GL_DEPTH_TEST);

      screenShader->use();
      screenShader->setInt("screenTexture", 0);
      glBindVertexArray(quadVAO);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texColorBuffer);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // Sandbox functions
    void DrawModel()
    {
      // Bind textures
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, diffuseMap);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, specularMap);

      // Set shader uniforms
      modelShader->use();
      modelShader->setMat4("projection", projection);
      modelShader->setMat4("view", view);
      modelShader->setInt("material.diffuse", 0);
      modelShader->setInt("material.specular", 1);
      modelShader->setFloat("material.shininess", 64.0f);
      modelShader->setVec3("light.position", lightPos);
      modelShader->setVec3("light.ambient", 1.0f, 1.0f, 1.0f);
      modelShader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
      modelShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
      modelShader->setVec3("viewPos", camera.Position);

      glm::mat4 model;
      modelShader->setMat4("model", model);

      // render the model
      cube->Draw(*modelShader);
    }

    void DrawLamp()
    {
      // also draw the lamp object
      lampShader->use();
      lampShader->setMat4("projection", projection);
      lampShader->setMat4("view", view);
      glm::mat4 model = glm::mat4();
      model = glm::translate(model, lightPos);
      model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
      lampShader->setMat4("model", model);

      lamp->Draw(*lampShader);
    }

  private:
    unsigned int framebuffer;
    unsigned int rbo;
    unsigned int texColorBuffer;

    unsigned int quadVBO;
    unsigned int quadVAO;

    Shader* modelShader;
    Shader* lampShader;
    Shader* screenShader;

    Model* cube;
    Model* lamp;

    unsigned int diffuseMap;
    unsigned int specularMap;

    // Matrices
    glm::mat4 view;
    glm::mat4 projection;
    
    // lighting
    glm::vec3 lightPos;

};
#endif
