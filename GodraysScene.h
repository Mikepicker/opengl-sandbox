#ifndef GODRAYS_SCENE_H
#define GODRAYS_SCENE_H

#include "Scene.h"
#include "Shader.h"
#include "Mesh.h"
#include "OBJImporter.h"

#include <iostream>
#include <vector>
#include <math.h>

class GodraysScene : public Scene
{
  public:
    GodraysScene(GLFWwindow* window, unsigned int width, unsigned int height)
      : Scene(window, width, height)
    {
      // Light position
      lightPos = glm::vec3(13.0f, 3.0f, 3.0f);

      // configure global opengl state
      glEnable(GL_DEPTH_TEST);

      // build and compile our shader zprogram
      modelShader = new Shader("basic_no_tex.vs", "basic_no_tex.fs");
      lampShader = new Shader("lamp.vs", "lamp.fs");
      godraysShader = new Shader("res/shaders/godrays/godrays.vs", "res/shaders/godrays/godrays.fs");

      // set up vertex data (and buffer(s)) and configure vertex attributes
      std::vector<Vertex> vertices;
      std::vector<unsigned int> indices;
      OBJImporter importer;
      importer.importOBJ("res/models/arena.obj", vertices, indices);

      // Model mesh
      modelMesh = new Mesh(vertices, indices);

      // Lamp mesh
      std::vector<Vertex> lampVertices;
      std::vector<unsigned int> lampIndices;
      importer.importOBJ("res/models/cube.obj", lampVertices, lampIndices);
      lampMesh = new Mesh(lampVertices, lampIndices);

      diffuseMap = loadTexture("res/textures/crate.png");

      // Generate and bind to FBO
      glGenFramebuffers(1, &framebuffer);
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

      // generate texture
      glGenTextures(1, &texColorBuffer);
      glBindTexture(GL_TEXTURE_2D, texColorBuffer);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
      processInput(window);

      // render
      // ------
      glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // view/projection transformations
      projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
      view = camera.GetViewMatrix();

      //----------------------FIRST PASS----------------------\\

      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
      glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);

      // Draw model
      DrawModel(true); 

      // Draw lamp
      DrawLamp();

      //----------------------SECOND PASS----------------------\\

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      // Draw model
      DrawModel(false); 

      //----------------------THIRD PASS----------------------\\

      glm::mat4 ortho = glm::ortho(0.0f, (float)windowWidth, 0.0f,(float)windowHeight, 0.1f, 100.0f);
      glm::vec4 clipLight = projection * view * glm::vec4(lightPos.x, lightPos.y, lightPos.z, 1.0);
      glm::vec3 ndcLight = clipLight / clipLight.w;

      godraysShader->use();
      godraysShader->setVec2("lightPositionOnScreen", glm::vec2((ndcLight.x + 1) / 2, (ndcLight.y + 1) / 2));
      godraysShader->setInt("firstPass", 0);
      godraysShader->setFloat("exposure", 0.0034f);
      godraysShader->setFloat("decay", 1.0f);
      godraysShader->setFloat("density", 0.84f);
      godraysShader->setFloat("weight", 5.65f);
      
      glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

      glBindVertexArray(quadVAO);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texColorBuffer);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      glDisable(GL_BLEND);
    }

    // Sandbox functions
    void DrawModel(bool firstPass)
    {
      // Bind textures
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, diffuseMap);

      // Set shader uniforms
      modelShader->use();
      modelShader->setMat4("projection", projection);
      modelShader->setMat4("view", view);
      
      if (firstPass) {
        modelShader->setVec3("material.diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
        modelShader->setVec3("material.specular", glm::vec3(0.0f, 0.0f, 0.0f));
      } else {
        modelShader->setVec3("material.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
        modelShader->setVec3("material.specular", glm::vec3(1.0f, 1.0f, 1.0f));
      }

      modelShader->setFloat("material.shininess", 128.0f);
      modelShader->setVec3("light.position", lightPos);
      modelShader->setVec3("light.ambient", 1.0f, 1.0f, 1.0f);
      modelShader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
      modelShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
      modelShader->setVec3("viewPos", camera.Position);

      glm::mat4 model;
      modelShader->setMat4("model", model);

      // render the model
      modelMesh->Draw(*modelShader);
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

      lampMesh->Draw(*lampShader);

    }

  private:
    unsigned int framebuffer;
    unsigned int rbo;
    unsigned int texColorBuffer;

    unsigned int quadVBO;
    unsigned int quadVAO;

    Shader* modelShader;
    Shader* lampShader;
    Shader* godraysShader;
    Mesh* modelMesh;
    Mesh* lampMesh;
    unsigned int diffuseMap;

    // Matrices
    glm::mat4 view;
    glm::mat4 projection;
    
    // lighting
    glm::vec3 lightPos;
    glm::vec3 skyColor = glm::vec3(0.1f, 0.3f, 0.8f);

};
#endif
