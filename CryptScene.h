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
      // Shader params
      basicParams.la = basicParams.ld = basicParams.ls = 0.5f;
      basicParams.s = 32;

      // Load models
      crypt = new CryptModel("res/models/crypt/crypt.obj");
    }

    void Draw()
    {
      Scene::Draw();

      if (lightFollowCamera)
        m_LightPos = camera.Position;

      // Bind Godrays
      m_Godrays->Bind();
        DrawOcclusionScene();
        if (!lightFollowCamera)
          DrawLamp();
      m_Godrays->Unbind();

      // 1. Compute Shadow Map
      m_ShadowMap->ComputeShadowMap(m_LightPos, *crypt);

      // 2. Render scene as normal using the generated depth/shadow map
      m_ShadowMap->Bind();
      DrawScene();
      if (!lightFollowCamera)
        DrawLamp();

      // Blend godrays
      m_Godrays->Draw(m_Projection, m_View, m_LightPos, m_GodraysParams);
      
      // Draw GUI
      DrawGUI();
    }

  private:

    // Models
    Model* crypt;

    // Lighting
    bool lightFollowCamera = false;

    // Shader params
    ShaderParams basicParams;
    bool softShadows = false;

    // Draw Scene
    void DrawScene()
    {
      SetShaderParams(basicParams);
      ((CryptModel*)crypt)->DrawCrypt(m_Projection, m_View, camera, m_LightPos, *m_UberShader); 
    }

    void DrawOcclusionScene()
    {
      ShaderParams p;
      p.la = p.ld = p.ls = p.s = 0.0f;
      SetShaderParams(p);
      ((CryptModel*)crypt)->DrawCrypt(m_Projection, m_View, camera, m_LightPos, *m_UberShader); 
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
          m_LightPos = camera.Position;
        ImGui::Text("Light Pos = %.3f %.3f %.3f", m_LightPos.x, m_LightPos.y, m_LightPos.z);
        ImGui::Text("Camera Pos = %.3f %.3f %.3f", camera.Position.x, camera.Position.y, camera.Position.z);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);


      ImGui::End();

      ImGui::Begin("Godrays");
        ImGui::SliderFloat("Exposure", &m_GodraysParams.exposure, 0.0f, 0.01f);
        ImGui::SliderFloat("Decay", &m_GodraysParams.decay, 0.0f, 1.0f);
        ImGui::SliderFloat("Density", &m_GodraysParams.density, 0.0f, 1.0f);
        ImGui::SliderFloat("Weight", &m_GodraysParams.weight, 0.0f, 10.0f);

        if (ImGui::Button("Light follow camera"))
          lightFollowCamera = !lightFollowCamera;

      ImGui::End();

      ImGui::Render();
      ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }
};

#endif
