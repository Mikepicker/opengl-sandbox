#ifndef GODRAYS_SCENE_H
#define GODRAYS_SCENE_H

#include "Scene.h"
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "Skybox.h"
#include "Godrays.h"

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
      lightPos = glm::vec3(4.0f, 4.0f, -4.0f);

      // configure global opengl state
      glEnable(GL_DEPTH_TEST);

      // build and compile our shader program
      modelShader = new Shader("res/shaders/basic/basic.vs", "res/shaders/basic/basic.fs");
      lampShader = new Shader("res/shaders/basic/lamp.vs", "res/shaders/basic/lamp.fs");

      // Load models
      tower = new Model("res/models/tower/tower.obj");
      lamp = new Model("res/models/cube.obj");

      // Godrays
      godrays = new Godrays(s_WindowWidth, s_WindowHeight);
      godraysParams.exposure = 0.0034f;
      godraysParams.decay = 0.97f;
      godraysParams.density = 0.84f;
      godraysParams.weight = 3.65f;

      // Generate skybox
      skybox = new Skybox();

      glEnable(GL_DEPTH_TEST);
    }

    void Draw()
    {
      Scene::Draw();

      // Draw GUI
      DrawGUI();

      // View/projection transformations
      projection = glm::perspective(glm::radians(camera.Zoom), (float)s_WindowWidth / (float)s_WindowHeight, 0.1f, 100.0f);
      view = camera.GetViewMatrix();

      //----------------------FIRST PASS----------------------\\

      // Bind Godrays and draw lamp + black models
      godrays->Bind();

      // Draw model
      SetShaderParams(true); 
      tower->Draw(*modelShader);

      // Draw lamp
      DrawLamp();

      //----------------------SECOND PASS----------------------\\

      godrays->Unbind();

      glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Draw model
      SetShaderParams(false); 
      tower->Draw(*modelShader);

      // Draw lamp
      DrawLamp();

      // Draw skybox as last
      skybox->Draw(projection, view);

      //----------------------THIRD PASS----------------------\\

      // Blend Godrays FBO with the scene
      godrays->Draw(projection, view, lightPos, godraysParams);

      // Render GUI
      ImGui::Render();
      ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // Imgui
    void DrawGUI()
    {
      ImGui_ImplGlfwGL3_NewFrame();

      ImGui::Text("Godrays Parameters");
      ImGui::SliderFloat("Exposure", &godraysParams.exposure, 0.0f, 0.01f);
      ImGui::SliderFloat("Decay", &godraysParams.decay, 0.0f, 1.0f);
      ImGui::SliderFloat("Density", &godraysParams.density, 0.0f, 1.0f);
      ImGui::SliderFloat("Weight", &godraysParams.weight, 0.0f, 10.0f);

      if (ImGui::Button("Set Light Here"))
        lightPos = camera.Position;
      ImGui::Text("Light Pos = %.3f %.3f %.3f", lightPos.x, lightPos.y, lightPos.z);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

      ImGui::Render();
      ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // Sandbox functions
    void SetShaderParams(bool firstPass)
    {
      // Set shader uniforms
      modelShader->use();
      modelShader->setMat4("projection", projection);
      modelShader->setMat4("view", view);
     
      // Set all black for the first pass (occluders map)
      if (firstPass) {
        modelShader->setVec3("material.diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
        modelShader->setVec3("material.specular", glm::vec3(0.0f, 0.0f, 0.0f));
        modelShader->setFloat("material.shininess", 0.0f);
        modelShader->setVec3("light.position", lightPos);
        modelShader->setVec3("light.ambient", 0.0f, 0.0f, 0.0f);
        modelShader->setVec3("light.diffuse", 0.0f, 0.0f, 0.0f);
        modelShader->setVec3("light.specular", 0.0f, 0.0f, 0.0f);

      } else {
        modelShader->setVec3("material.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
        modelShader->setVec3("material.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        modelShader->setFloat("material.shininess", 32.0f);
        modelShader->setVec3("light.position", lightPos);
        modelShader->setVec3("light.ambient", 1.0f, 1.0f, 1.0f);
        modelShader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        modelShader->setVec3("light.specular", 0.5f, 0.5f, 0.5f);
      }

      modelShader->setVec3("viewPos", camera.Position);

      glm::mat4 model;
      modelShader->setMat4("model", model);
    }

    void DrawLamp()
    {
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
    Shader* modelShader;
    Shader* lampShader;

    Model* tower;
    Model* lamp;

    // Godrays settings
    Godrays* godrays;
    GodraysParams godraysParams;

    // Matrices
    glm::mat4 view;
    glm::mat4 projection;
    
    // Lighting
    glm::vec3 lightPos;
    glm::vec3 skyColor = glm::vec3(0.3f, 0.3f, 0.3f);

    // Skybox
    Skybox* skybox;

};
#endif
