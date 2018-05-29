#ifndef SPONZA_SCENE_H
#define SPONZA_SCENE_H

#include "Scene.h"
#include "Model.h"
#include "Skybox.h"

class SponzaScene : public Scene
{
  public:
    SponzaScene(GLFWwindow* window, unsigned int width, unsigned int height)
      : Scene(window, width, height)
    {
      sponza = new Model("res/models/sponza/sponza.obj");
      model = glm::mat4();
      model = glm::scale(model, glm::vec3(0.05f));
      //sponza = new Model("res/models/crypt/crypt.obj");
      skybox = new Skybox();

      // Shader params
      shaderParams.la = shaderParams.ld = shaderParams.ls = 0.5f;
      shaderParams.s = 32;

      m_LightPos = glm::vec3(30.0f, 35.0f, 0.0f);

      m_ShadowMap->ComputeShadowMap(*sponza, model, m_LightPos);
    }

    void Draw()
    {
      Scene::Draw();
      //m_LightPos = camera.Position;
      
      m_UberShader->use();
      // set lighting uniforms
      m_UberShader->setFloat("far_plane", m_ShadowMap->far_plane);
      m_ShadowMap->Bind();

      if (!debugShadows)
      {
        SetShaderParams(shaderParams);
        m_UberShader->setMat4("model", model);
        m_UberShader->setBool("hasShadows", shadowsEnabled);
        m_UberShader->setFloat("bias", bias);
        m_UberShader->setFloat("time", glfwGetTime());

        sponza->Draw(*m_UberShader);
        skybox->Draw(m_Projection, m_View);
      }

      DrawGUI();
    }

  private:
    Model* sponza;
    Skybox* skybox;
    ShaderParams shaderParams;
    bool shadowsEnabled = true;
    bool debugShadows = false;
    glm::mat4 model;

    // Shadow map
    float bias = 0.05;

    // Imgui
    void DrawGUI()
    {
      ImGui_ImplGlfwGL3_NewFrame();

      ImGui::Begin("Materials");

      ImGui::Text("Shader Parameters");
      ImGui::SliderFloat("Light Ambient", &shaderParams.la, 0.0f, 1.0f);
      ImGui::SliderFloat("Light Diffuse", &shaderParams.ld, 0.0f, 1.0f);
      ImGui::SliderFloat("Light Specular", &shaderParams.ls, 0.0f, 1.0f);
      ImGui::SliderInt("Shininess", &shaderParams.s, 0.0f, 128.0f);
      ImGui::SliderFloat("Bias", &bias, 0.05f, 1.0f);

      if (ImGui::Button("Debug Shadows"))
        debugShadows = !debugShadows;

      if (ImGui::Button("Compile Shader"))
      {
        m_UberShader = new Shader("res/shaders/ubershader.vs", "res/shaders/ubershader.fs");
      }

      if (ImGui::Button("Toggle Shadows"))
        shadowsEnabled = !shadowsEnabled;

      if (ImGui::Button("Toggle Soft Shadows"))
        m_SoftShadows = !m_SoftShadows;

      if (ImGui::Button("Set Light Here"))
      {
        m_LightPos = camera.Position;
        m_ShadowMap->ComputeShadowMap(*sponza, model, m_LightPos);
      }

      ImGui::Text("Light Pos = %.3f %.3f %.3f", m_LightPos.x, m_LightPos.y, m_LightPos.z);
      ImGui::Text("Camera Pos = %.3f %.3f %.3f", camera.Position.x, camera.Position.y, camera.Position.z);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);


      ImGui::End();

      ImGui::Render();
      ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }
};

#endif
