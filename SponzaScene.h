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
      skybox = new Skybox();

      // Shader params
      shaderParams.la = shaderParams.ld = shaderParams.ls = 0.5f;
      shaderParams.s = 32;
    }

    void Draw()
    {
      Scene::Draw();
      m_LightPos = camera.Position;

      SetShaderParams(shaderParams);
      glm::mat4 model;
      model = glm::scale(model, glm::vec3(0.05f));
      m_UberShader->setMat4("model", model);
      m_UberShader->setBool("hasNormalMap", normalsEnabled);
      m_UberShader->setBool("hasShadows", false);

      sponza->Draw(*m_UberShader);

      skybox->Draw(m_Projection, m_View);

      DrawGUI();
    }

  private:
    Model* sponza;
    Skybox* skybox;
    ShaderParams shaderParams;
    bool normalsEnabled = true;

    // Imgui
    void DrawGUI()
    {
      ImGui_ImplGlfwGL3_NewFrame();

      ImGui::Begin("Materials");

        ImGui::Text("Shader Parameters");
        ImGui::SliderFloat("Basic Light Ambient", &shaderParams.la, 0.0f, 1.0f);
        ImGui::SliderFloat("Basic Light Diffuse", &shaderParams.ld, 0.0f, 1.0f);
        ImGui::SliderFloat("Basic Light Specular", &shaderParams.ls, 0.0f, 1.0f);
        ImGui::SliderInt("Basic Shininess", &shaderParams.s, 0.0f, 128.0f);

        if (ImGui::Button("Compile Shader"))
        {
          m_UberShader = new Shader("res/shaders/ubershader.vs", "res/shaders/ubershader.fs");
        }

        if (ImGui::Button("Toggle Normals"))
          normalsEnabled = !normalsEnabled;

        if (ImGui::Button("Set Light Here"))
          m_LightPos = camera.Position;
        ImGui::Text("Light Pos = %.3f %.3f %.3f", m_LightPos.x, m_LightPos.y, m_LightPos.z);
        ImGui::Text("Camera Pos = %.3f %.3f %.3f", camera.Position.x, camera.Position.y, camera.Position.z);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);


      ImGui::End();
    
      ImGui::Render();
      ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }
};

#endif
