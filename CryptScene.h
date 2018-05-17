#ifndef CRYPT_SCENE_H
#define CRYPT_SCENE_H

#include "Scene.h"
#include "Model.h"
#include "CryptModel.h"
#include "Shader.h"

class CryptScene : Scene
{
  public:
    CryptScene(GLFWwindow* window, unsigned int width, unsigned int height)
      : Scene(window, width, height)
    {
      // Load shaders
      lampShader = new Shader("res/shaders/basic/lamp.vs", "res/shaders/basic/lamp.fs");

      // Load models
      crypt = new CryptModel("res/models/crypt/crypt.obj");
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

      lightPos = camera.Position;
      /*lightPos.x = sin(glfwGetTime()) * 10;
      lightPos.z = cos(glfwGetTime()) * 10;*/

      // Draw crypt
      crypt->Draw(projection, view, camera, lightPos); 
    }

  private:

    // Shaders
    Shader* lampShader;

    // Models
    CryptModel* crypt;
    Model* lamp;

    // Matrices
    glm::mat4 view;
    glm::mat4 projection;

    // Lighting
    glm::vec3 lightPos = glm::vec3(4.0f, 4.0f, -4.0f);
};

#endif
