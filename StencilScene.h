#ifndef STENCIL_SCENE_H
#define STENCIL_SCENE_H

#include "Scene.h"
#include "Shader.h"
#include "Model.h"

#include <iostream>
#include <vector>

class StencilScene : public Scene
{
  public:
    StencilScene(GLFWwindow* window, unsigned int width, unsigned int height)
      : Scene(window, width, height)
    {
      // Light position
      lightPos = glm::vec3(1.2f, 1.0f, 2.0f);

      // configure global opengl state
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_STENCIL_TEST);

      // build and compile our shader program
      modelShader = new Shader("res/shaders/basic/basic.vs", "res/shaders/basic/basic.fs");
      outlineShader = new Shader("res/shaders/outline/outline.vs", "res/shaders/outline/outline.fs");
      lampShader = new Shader("res/shaders/basic/lamp.vs", "res/shaders/basic/lamp.fs");

      // Load models
      character = new Model("res/models/militia/militia.obj");
      outline = new Model("res/models/militia/militia.obj");
      lamp = new Model("res/models/cube.obj");

    }

    void Draw()
    {
      Scene::Draw();

      // render
      // ------
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      // view/projection transformations
      projection = glm::perspective(glm::radians(camera.Zoom), (float)s_WindowWidth / (float)s_WindowHeight, 0.1f, 100.0f);
      view = camera.GetViewMatrix();

      // Draw model
      drawWithOutline(); 

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

    // Sandbox functions
    void drawWithOutline()
    {
      // Set stencil buffer to draw base model
      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
      glStencilMask(0xFF);

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
      model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0, 1.0f, 0));
      modelShader->setMat4("model", model);
      
      // render the model
      character->Draw(*modelShader);

      // OUTLINE
      glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      glStencilMask(0x00);
      glDisable(GL_DEPTH_TEST);

      float scale = 1.02f;
      model = glm::mat4();
      model = glm::translate(model, glm::vec3(0, 1-scale, 0));
      model = glm::scale(model, glm::vec3(scale));
      model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0, 1.0f, 0));

      outlineShader->use();
      outlineShader->setMat4("projection", projection);
      outlineShader->setMat4("view", view);
      outlineShader->setMat4("model", model);

      outline->Draw(*outlineShader);

      glStencilMask(0xFF);
      glEnable(GL_DEPTH_TEST);

    }

  private:
    Shader* modelShader;
    Shader* outlineShader;
    Shader* lampShader;

    Model* character;
    Model* outline;
    Model* lamp;

    unsigned int diffuseMap;

    // Matrices
    glm::mat4 view;
    glm::mat4 projection;
    
    // lighting
    glm::vec3 lightPos;

};
#endif
