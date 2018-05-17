#ifndef GODRAYS_SCENE_H
#define GODRAYS_SCENE_H

#include "Scene.h"
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "Skybox.h"

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
      godraysShader = new Shader("res/shaders/godrays/godrays.vs", "res/shaders/godrays/godrays.fs");

      // Load models
      tower = new Model("res/models/tower/tower.obj");
      lamp = new Model("res/models/cube.obj");

      // Generate and bind to FBO
      generateFBORBO(framebuffer, texColorBuffer, rbo);
      
      // Create quad VAO
      generateQuad(quadVAO, quadVBO); 

      // Generate skybox
      skybox = new Skybox();
    }

    void Draw()
    {
      // per-frame time logic
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      // input
      processInput();

      // Draw GUI
      DrawGUI();

      // view/projection transformations
      projection = glm::perspective(glm::radians(camera.Zoom), (float)s_WindowWidth / (float)s_WindowHeight, 0.1f, 100.0f);
      view = camera.GetViewMatrix();

      //----------------------FIRST PASS----------------------\\

      // FBO is downsampled (+ performance)
      glViewport(0, 0, s_WindowWidth/4, s_WindowHeight/4);

      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
      glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);

      // Draw model
      DrawModel(true); 

      // Draw lamp
      DrawLamp();

      glViewport(0, 0, s_WindowWidth, s_WindowHeight);

      //----------------------SECOND PASS----------------------\\

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Draw model
      DrawModel(false); 

      // Draw lamp
      DrawLamp();

      // Draw skybox as last
      skybox->Draw(projection, view);

      //----------------------THIRD PASS----------------------\\

      glm::vec4 clipLight = projection * view * glm::vec4(lightPos.x, lightPos.y, lightPos.z, 1.0);
      glm::vec3 ndcLight = clipLight / clipLight.w;

      godraysShader->use();

      godraysShader->setVec2("lightPositionOnScreen", glm::vec2((ndcLight.x + 1) / 2, (ndcLight.y + 1) / 2));
      godraysShader->setInt("firstPass", 0);
      godraysShader->setFloat("exposure", exposure);
      godraysShader->setFloat("decay", decay);
      godraysShader->setFloat("density", density);
      godraysShader->setFloat("weight", weight);
      
      glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

      glBindVertexArray(quadVAO);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texColorBuffer);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      glDisable(GL_BLEND);

      // Render GUI
      ImGui::Render();
      ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // Imgui
    void DrawGUI()
    {
      ImGui_ImplGlfwGL3_NewFrame();

      ImGui::Text("Godrays Parameters!");
      ImGui::SliderFloat("Exposure", &exposure, 0.0f, 0.01f);
      ImGui::SliderFloat("Decay", &decay, 0.0f, 1.0f);
      ImGui::SliderFloat("Density", &density, 0.0f, 1.0f);
      ImGui::SliderFloat("Weight", &weight, 0.0f, 10.0f);

      if (ImGui::Button("Set Light Here"))
        lightPos = camera.Position;
      ImGui::Text("Light Pos = %.3f %.3f %.3f", lightPos.x, lightPos.y, lightPos.z);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

      ImGui::Text("Test");
      ImGui::Render();
      ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // Sandbox functions
    void DrawModel(bool firstPass)
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

      tower->Draw();
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

      lamp->Draw();
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

    Model* tower;
    Model* lamp;

    // Godrays settings
    float exposure = 0.0034f;
    float decay = 0.97f;
    float density = 0.84f;
    float weight = 3.65f;

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
