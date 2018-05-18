#ifndef CRYPT_SCENE_H
#define CRYPT_SCENE_H

#include "Scene.h"
#include "Model.h"
#include "CryptModel.h"
#include "Shader.h"
#include "ShadowMap.h"

class CryptScene : Scene
{
  public:
    CryptScene(GLFWwindow* window, unsigned int width, unsigned int height)
      : Scene(window, width, height)
    {
      // Load shaders
      lampShader = new Shader("res/shaders/basic/lamp.vs", "res/shaders/basic/lamp.fs");
      shadowMapShader = new Shader("res/shaders/shadow/shadowmap.vs", "res/shaders/shadow/shadowmap.fs");

      // Shader params
      basicParams.la = basicParams.ld = basicParams.ls = 0.5f;
      basicParams.s = 32;
      normalParams.la = normalParams.ld = normalParams.ls = 0.5f;
      normalParams.s = 32;

      // Load models
      crypt = new CryptModel("res/models/crypt/crypt.obj");
      lamp = new Model("res/models/cube.obj");
      test = new Model("res/models/crypt/crypt.obj");

      // Global OpenGL setting
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);

      // ShadowMap
      shadowMap = new ShadowMap();
    }

    void Draw()
    {
      // Per-frame time logic
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      // Input
      processInput();

      // Projection - View matrices
      projection = glm::perspective(glm::radians(camera.Zoom), (float)s_WindowWidth / (float)s_WindowHeight, 0.1f, 100.0f);
      view = camera.GetViewMatrix();

      // Clear color and buffers
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // 1. Compute Shadow Map
      shadowMap->ComputeShadowMap(lightPos, *test);

      // 2. Render scene as normal using the generated depth/shadow map
      glViewport(0, 0, s_WindowWidth, s_WindowHeight);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      shadowMapShader->use();
      shadowMapShader->setMat4("projection", projection);
      shadowMapShader->setMat4("view", view);
      shadowMapShader->setMat4("model", glm::mat4());
      shadowMapShader->setBool("softShadows", softShadows);

      // Set light uniforms
      shadowMapShader->setVec3("viewPos", camera.Position);
      shadowMapShader->setVec3("lightPos", lightPos);
      shadowMapShader->setMat4("lightSpaceMatrix", shadowMap->lightSpaceMatrix);
      shadowMapShader->setInt("diffuseTexture", 0);
      shadowMapShader->setInt("shadowMap", 2);
      shadowMap->Bind();
      test->Draw();
      
      // Draw GUI
      DrawGUI();
    }

  private:

    // Shaders
    Shader* lampShader;
    Shader* shadowMapShader;
    Shader* shadowDepthShader;
    Shader* debugShadowMapShader;

    // Models
    CryptModel* crypt;
    Model* lamp;
    Model* test;

    // Matrices
    glm::mat4 view;
    glm::mat4 projection;

    // Lighting
    glm::vec3 lightPos = glm::vec3(0.0f, 2.5f, -9.0f);

    // Shader params
    ShaderParams normalParams, basicParams;
    bool softShadows = false;

    // ShadowMap
    ShadowMap* shadowMap;
    
    // Draw Scene
    void DrawScene()
    {
      // View/projection transformations
      /*projection = glm::perspective(glm::radians(camera.Zoom), (float)s_WindowWidth / (float)s_WindowHeight, 0.1f, 100.0f);
      view = camera.GetViewMatrix();*/

      // Draw crypt
      crypt->Draw(projection, view, camera, lightPos, basicParams, normalParams); 
    }

    // Imgui
    void DrawGUI()
    {
      ImGui_ImplGlfwGL3_NewFrame();

      ImGui::Text("Normal Parameters");
      ImGui::SliderFloat("Normal Light Ambient", &normalParams.la, 0.0f, 1.0f);
      ImGui::SliderFloat("Normal Light Diffuse", &normalParams.ld, 0.0f, 1.0f);
      ImGui::SliderFloat("Normal Light Specular", &normalParams.ls, 0.0f, 1.0f);
      ImGui::SliderInt("Normal Shininess", &normalParams.s, 0.0f, 128.0f);

      ImGui::Text("Basic Parameters");
      ImGui::SliderFloat("Basic Light Ambient", &basicParams.la, 0.0f, 1.0f);
      ImGui::SliderFloat("Basic Light Diffuse", &basicParams.ld, 0.0f, 1.0f);
      ImGui::SliderFloat("Basic Light Specular", &basicParams.ls, 0.0f, 1.0f);
      ImGui::SliderInt("Basic Shininess", &basicParams.s, 0.0f, 128.0f);

      if (ImGui::Button("Toggle Soft Shadows"))
        softShadows = !softShadows;
      ImGui::Text("Soft Shadows %s", softShadows ? "On" : "Off");

      if (ImGui::Button("Set Light Here"))
        lightPos = camera.Position;
      ImGui::Text("Light Pos = %.3f %.3f %.3f", lightPos.x, lightPos.y, lightPos.z);
      ImGui::Text("Camera Pos = %.3f %.3f %.3f", camera.Position.x, camera.Position.y, camera.Position.z);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

      ImGui::Render();
      ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }
};

#endif
