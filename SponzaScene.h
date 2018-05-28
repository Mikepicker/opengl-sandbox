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
      //sponza = new Model("res/models/crypt/crypt.obj");
      skybox = new Skybox();

      // Shader params
      shaderParams.la = shaderParams.ld = shaderParams.ls = 0.5f;
      shaderParams.s = 32;

      m_LightPos = glm::vec3(30.0f, 35.0f, 0.0f);

      // Shadow depth shader
      depthShader = new Shader("res/shaders/shadow/omni.vs", "res/shaders/shadow/omni.fs", "res/shaders/shadow/omni.gs");
      //depthShader = new Shader("res/shaders/shadow/omni.vs", "res/shaders/shadow/omni.fs");

      // configure depth map FBO
      // -----------------------
      glGenFramebuffers(1, &depthMapFBO);
      // create depth cubemap texture
      glGenTextures(1, &depthCubemap);
      glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
      for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

      // attach depth texture as FBO's depth buffer
      glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);

      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      ComputeShadowMap();
    }

    void Draw()
    {
      Scene::Draw();
      //m_LightPos = camera.Position;

      glm::mat4 model;
      model = glm::scale(model, glm::vec3(scale));
      
      // 2. render scene as normal
      // -------------------------
      glViewport(0, 0, s_WindowWidth, s_WindowHeight);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      m_UberShader->use();
      // set lighting uniforms
      m_UberShader->setFloat("far_plane", far_plane);
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

      if (!debugShadows)
      {
        SetShaderParams(shaderParams);
        m_UberShader->setMat4("model", model);
        m_UberShader->setBool("hasShadows", shadowsEnabled);
        m_UberShader->setFloat("bias", bias);

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
    float scale = 0.05f;

    // Shadow map
    Shader* depthShader;
    unsigned int depthMapFBO;
    unsigned int depthCubemap;
    float near_plane = 1.0f;
    float far_plane = 100.0f;
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
        ComputeShadowMap();
      }

      ImGui::Text("Light Pos = %.3f %.3f %.3f", m_LightPos.x, m_LightPos.y, m_LightPos.z);
      ImGui::Text("Camera Pos = %.3f %.3f %.3f", camera.Position.x, camera.Position.y, camera.Position.z);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);


      ImGui::End();

      ImGui::Render();
      ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ComputeShadowMap()
    {
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glm::mat4 model;
      model = glm::scale(model, glm::vec3(scale));

      // 0. create depth cubemap transformation matrices
      // -----------------------------------------------
      glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
      std::vector<glm::mat4> shadowTransforms;
      shadowTransforms.push_back(shadowProj * glm::lookAt(m_LightPos, m_LightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(m_LightPos, m_LightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(m_LightPos, m_LightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(m_LightPos, m_LightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(m_LightPos, m_LightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(m_LightPos, m_LightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

      // 1. render scene to depth cubemap
      // --------------------------------
      glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
      glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
      glClear(GL_DEPTH_BUFFER_BIT);
      depthShader->use();
      for (unsigned int i = 0; i < 6; ++i)
        depthShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
      depthShader->setMat4("model", model);
      depthShader->setFloat("far_plane", far_plane);
      depthShader->setVec3("lightPos", m_LightPos);
      sponza->Draw(*depthShader);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

#endif
