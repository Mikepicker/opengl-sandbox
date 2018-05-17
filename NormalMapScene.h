#ifndef NORMAL_MAP_SCENE_H
#define NORMAL_MAP_SCENE_H

#include "Scene.h"
#include "Model.h"
#include "Shader.h"

class NormalMapScene : Scene
{
  public:
    NormalMapScene(GLFWwindow* window, unsigned int width, unsigned int height)
      : Scene(window, width, height)
    {
      // Load shaders
      nmShader = new Shader("res/shaders/normalmap/normalmap.vs", "res/shaders/normalmap/normalmap.fs");
      lampShader = new Shader("res/shaders/basic/lamp.vs", "res/shaders/basic/lamp.fs");

      // Load models
      cube = new Model("res/models/cube_norm.obj");
      lamp = new Model("res/models/cube.obj");

      // Global OpenGL setting
      glEnable(GL_DEPTH_TEST);
    }

    void Draw()
    {
      // Per-frame time logic
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      // Input
      processInput();

      // View/projection transformations
      projection = glm::perspective(glm::radians(camera.Zoom), (float)s_WindowWidth / (float)s_WindowHeight, 0.1f, 100.0f);
      view = camera.GetViewMatrix();

      // Clear color and buffers
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      //lightPos = camera.Position;
      lightPos.x = sin(glfwGetTime()) * 10;
      lightPos.z = cos(glfwGetTime()) * 10;

      // Set uniforms
      glm::mat4 model;
      nmShader->use();
      nmShader->setInt("diffuseMap", 0);
      nmShader->setInt("normalMap", 1);
      nmShader->setMat4("model", model);
      nmShader->setMat4("projection", projection);
      nmShader->setMat4("view", view);
      nmShader->setVec3("viewPos", camera.Position);
      nmShader->setVec3("lightPos", lightPos);

      // Draw cube model
      cube->Draw(); 
    }

  private:

    // Shaders
    Shader* nmShader;
    Shader* lampShader;

    // Models
    Model* cube;
    Model* lamp;

    // Matrices
    glm::mat4 view;
    glm::mat4 projection;

    // Lighting
    glm::vec3 lightPos = glm::vec3(4.0f, 4.0f, -4.0f);
};

#endif
