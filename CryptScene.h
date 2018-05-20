#ifndef CRYPT_SCENE_H
#define CRYPT_SCENE_H

#include "Scene.h"
#include "Model.h"
#include "CryptModel.h"
#include "Shader.h"
#include "ShadowMap.h"
#include "Godrays.h"

class CryptScene : Scene
{
  public:
    CryptScene(GLFWwindow* window, unsigned int width, unsigned int height)
      : Scene(window, width, height)
    {
      // Load shaders
      lampShader = new Shader("res/shaders/basic/lamp.vs", "res/shaders/basic/lamp.fs");
      uberShader = new Shader("res/shaders/ubershader.vs", "res/shaders/ubershader.fs");

      // Shader params
      basicParams.la = basicParams.ld = basicParams.ls = 0.5f;
      basicParams.s = 32;

      // Load models
      crypt = new CryptModel("res/models/crypt/crypt.obj");
      lamp = new Model("res/models/cube.obj");

      // Global OpenGL setting
      glEnable(GL_DEPTH_TEST);
      //glEnable(GL_CULL_FACE);

      // ShadowMap
      shadowMap = new ShadowMap(width, height);

      // Godrays
      godrays = new Godrays(s_WindowWidth, s_WindowHeight);
      godraysParams.exposure = 0.0034f;
      godraysParams.decay = 1.0f;
      godraysParams.density = 0.84f;
      godraysParams.weight = 3.65f;
    }

    void Draw()
    {
      Scene::Draw();

      // Projection - View matrices
      projection = glm::perspective(glm::radians(camera.Zoom), (float)s_WindowWidth / (float)s_WindowHeight, 0.1f, 100.0f);
      view = camera.GetViewMatrix();

      if (lightFollowCamera)
        lightPos = camera.Position;

      // Bind Godrays
      godrays->Bind();
        DrawOcclusionScene();
        if (!lightFollowCamera)
          DrawLamp();
      godrays->Unbind();

      // 1. Compute Shadow Map
      shadowMap->ComputeShadowMap(lightPos, *crypt);

      // 2. Render scene as normal using the generated depth/shadow map
      shadowMap->Bind();
      DrawScene();
      if (!lightFollowCamera)
        DrawLamp();

      // Blend godrays
      godrays->Draw(projection, view, lightPos, godraysParams);
      
      // Draw GUI
      DrawGUI();
    }

  private:

    // Shaders
    Shader* lampShader;
    Shader* uberShader;

    // Models
    Model* crypt;
    Model* lamp;

    // Matrices
    glm::mat4 view;
    glm::mat4 projection;

    // Lighting
    glm::vec3 lightPos = glm::vec3(3.0f, 6.5f, -14.5f);
    bool lightFollowCamera = false;

    // Shader params
    ShaderParams basicParams;
    bool softShadows = false;

    // ShadowMap
    ShadowMap* shadowMap;

    // Godrays
    Godrays* godrays;
    GodraysParams godraysParams;
    
    // Draw Scene
    void DrawScene()
    {
      // View/projection transformations
      /*projection = glm::perspective(glm::radians(camera.Zoom), (float)s_WindowWidth / (float)s_WindowHeight, 0.1f, 100.0f);
      view = camera.GetViewMatrix();*/

      // Draw crypt
      //crypt->Draw(projection, view, camera, lightPos, basicParams, normalParams); 

      uberShader->use();
      uberShader->setMat4("projection", projection);
      uberShader->setMat4("view", view);
      uberShader->setMat4("model", glm::mat4());

      // Shadows
      uberShader->setBool("softShadows", softShadows);
      uberShader->setInt("shadowMap", 2);

      // Set light uniforms
      uberShader->setVec3("viewPos", camera.Position);
      uberShader->setVec3("lightPos", lightPos);
      uberShader->setMat4("lightSpaceMatrix", shadowMap->lightSpaceMatrix);

      // Material
      uberShader->setInt("material.diffuse", 0);
      uberShader->setInt("normalMap", 1);
      uberShader->setFloat("material.shininess", 64.0f);

      // Light
      uberShader->setVec3("light.position", lightPos);
      uberShader->setVec3("light.ambient", glm::vec3(basicParams.la));
      uberShader->setVec3("light.diffuse", glm::vec3(basicParams.ld));
      uberShader->setVec3("light.specular", glm::vec3(basicParams.ls));

      ((CryptModel*)crypt)->DrawCrypt(projection, view, camera, lightPos, *uberShader); 
    }

    void DrawOcclusionScene()
    {
      uberShader->use();
      uberShader->setMat4("projection", projection);
      uberShader->setMat4("view", view);
      uberShader->setMat4("model", glm::mat4());

      // Shadows
      uberShader->setBool("softShadows", softShadows);
      uberShader->setInt("shadowMap", 2);

      // Set light uniforms
      uberShader->setVec3("viewPos", camera.Position);
      uberShader->setVec3("lightPos", lightPos);
      uberShader->setMat4("lightSpaceMatrix", shadowMap->lightSpaceMatrix);

      // Material
      uberShader->setInt("material.diffuse", 0);
      uberShader->setInt("normalMap", 1);
      uberShader->setFloat("material.shininess", 0.0f);

      // Light
      uberShader->setVec3("light.position", lightPos);
      uberShader->setVec3("light.ambient", glm::vec3(0.0f));
      uberShader->setVec3("light.diffuse", glm::vec3(0.0f));
      uberShader->setVec3("light.specular", glm::vec3(0.0f));

      // Disable normal mapping
      uberShader->setBool("hasNormalMap", false);

      ((CryptModel*)crypt)->DrawCrypt(projection, view, camera, lightPos, *uberShader); 
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

    // Imgui
    void DrawGUI()
    {
      ImGui_ImplGlfwGL3_NewFrame();

      ImGui::Begin("Materials");

        ImGui::Text("Shader Parameters");
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


      ImGui::End();

      ImGui::Begin("Godrays");
        ImGui::SliderFloat("Exposure", &godraysParams.exposure, 0.0f, 0.01f);
        ImGui::SliderFloat("Decay", &godraysParams.decay, 0.0f, 1.0f);
        ImGui::SliderFloat("Density", &godraysParams.density, 0.0f, 1.0f);
        ImGui::SliderFloat("Weight", &godraysParams.weight, 0.0f, 10.0f);

        if (ImGui::Button("Light follow camera"))
          lightFollowCamera = !lightFollowCamera;

      ImGui::End();

      ImGui::Render();
      ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }
};

#endif
