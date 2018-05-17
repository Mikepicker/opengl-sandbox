#ifndef TERRAIN_SCENE_H
#define TERRAIN_SCENE_H

#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Terrain.h"
#include "Skybox.h"

class TerrainScene : Scene
{
  public:
    TerrainScene(GLFWwindow* window, unsigned int width, unsigned int height)
      : Scene(window, width, height)

    {
      // Init terrain
      terrain = new Terrain("res/textures/heightmap_mountain.jpg", "res/textures/grass.jpg", "res/textures/snow.jpg", "res/textures/dirt.png");
      terrainShader = new Shader("res/shaders/terrain/terrain.vs", "res/shaders/terrain/terrain.fs");

      // Load lamp
      lampShader = new Shader("res/shaders/basic/lamp.vs", "res/shaders/basic/lamp.fs");
      lamp = new Model("res/models/cube.obj");

      // Generate skybox
      skybox = new Skybox();

      glEnable(GL_DEPTH_TEST);
    }

    void Draw()
    {
      // per-frame time logic
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      processInput();


      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // View - Projection
      view = camera.GetViewMatrix();
      projection = glm::perspective(glm::radians(camera.Zoom), (float)s_WindowWidth / (float)s_WindowHeight, 0.1f, 100.0f);

      terrainShader->use();
      glm::mat4 model;

      // Textures: 0->heightmap | 1->grass | 2->snow | 3->dirt
      terrainShader->setInt("heightmap", 0);
      terrainShader->setInt("grass", 1);
      terrainShader->setInt("snow", 2);
      terrainShader->setInt("dirt", 3);

      //model = glm::scale(model, glm::vec3(4.0f));
      terrainShader->setMat4("view", view);
      terrainShader->setMat4("projection", projection);
      terrainShader->setMat4("model", model);

      // Debug
      terrainShader->setFloat("elevation", elevation);

      lightPos.x = sin(glfwGetTime()) * 10;
      lightPos.z = cos(glfwGetTime()) * 10;
      //lightPos = camera.Position;
      terrainShader->setVec3("light.position", lightPos.x, lightPos.y, lightPos.z);
      terrainShader->setVec3("light.ambient", 0.5f, 0.5f, 0.5f);
      terrainShader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);

      terrain->Draw();

      DrawLamp();

      // Draw Skybox
      skybox->Draw(projection, view);

      DrawGUI();
    }

  private:
    Terrain* terrain;
    Model* lamp;
    Shader* terrainShader;
    Shader* lampShader;

    Skybox* skybox;

    glm::vec3 lightPos = glm::vec3(0.0f, 0.5f, 0.0f);

    glm::mat4 projection;
    glm::mat4 view;

    float elevation = 0.1f;

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

    // Imgui
    void DrawGUI()
    {
      ImGui_ImplGlfwGL3_NewFrame();

      ImGui::Text("Terrain parameters");
      ImGui::SliderFloat("Elevation", &elevation, 0.01f, 1.0f);

      ImGui::Text("Light Pos = %.3f %.3f %.3f", lightPos.x, lightPos.y, lightPos.z);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

      ImGui::Render();
      ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }

};

#endif
