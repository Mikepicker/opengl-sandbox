#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include "dep/glm/glm.hpp"
#include "dep/glm/gtc/matrix_transform.hpp"
#include "dep/glm/gtc/type_ptr.hpp"
#include "dep/imgui/imgui.h"
#include "dep/imgui/imgui_impl_glfw_gl3.h"

#include "Camera.h"
#include "Godrays.h"
#include "ShadowMap.h"

// Static variables for GLFW (they must be updated by GLFW callbacks!)
GLFWwindow* s_Window;
unsigned int s_WindowWidth;
unsigned int s_WindowHeight;

float lastX = 0.0f;
float lastY = 0.0f;
bool firstMouse = true;
bool captureMouse = true;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  if (!captureMouse)
    return;

  if (firstMouse)
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if (captureMouse)
    camera.ProcessMouseScroll(yoffset);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
  {
    captureMouse = !captureMouse;
    glfwSetInputMode(window, GLFW_CURSOR, captureMouse ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
  }
}

enum ShaderMode
{
  NORMAL = 0,
  OCCLUSION = 1
};

class Scene
{
  public:
    Scene(GLFWwindow* window, unsigned int width, unsigned int height)
    {
      s_Window = window;
      s_WindowWidth = width;
      s_WindowHeight = height;

      lastX = width / 2.0f;
      lastY = height / 2.0f;

      glfwSetFramebufferSizeCallback(s_Window, framebuffer_size_callback);
      glfwSetCursorPosCallback(s_Window, mouse_callback);
      glfwSetScrollCallback(s_Window, scroll_callback);
      glfwSetMouseButtonCallback(s_Window, mouse_button_callback);

      // tell GLFW to capture our mouse
      glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

      // Imgui
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO(); (void)io;
      ImGui_ImplGlfwGL3_Init(window, false);

      ImGui::StyleColorsDark();

      // Load shaders
      m_LampShader = new Shader("res/shaders/basic/lamp.vs", "res/shaders/basic/lamp.fs");
      m_UberShader = new Shader("res/shaders/ubershader.vs", "res/shaders/ubershader.fs");

      // Load lamp model
      m_Lamp = new Model("res/models/cube.obj");

      // ShadowMap
      m_ShadowMap = new ShadowMap(width, height);

      // Godrays
      m_Godrays = new Godrays(s_WindowWidth, s_WindowHeight);
      m_GodraysParams.exposure = 0.0034f;
      m_GodraysParams.decay = 1.0f;
      m_GodraysParams.density = 0.84f;
      m_GodraysParams.weight = 3.65f;

      // OpenGL settings
      glEnable(GL_DEPTH_TEST);
      //glEnable(GL_CULL_FACE);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    virtual void Draw() {
      // Per-frame time logic
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      // Input
      processInput();

      // Projection - View matrices
      m_Projection = glm::perspective(glm::radians(camera.Zoom), (float)s_WindowWidth / (float)s_WindowHeight, 0.1f, 1000.0f);
      m_View = camera.GetViewMatrix();

      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void SetShaderParams(ShaderParams params)
    {
      m_UberShader->use();
      m_UberShader->setMat4("projection", m_Projection);
      m_UberShader->setMat4("view", m_View);
      m_UberShader->setMat4("model", glm::mat4());

      // Shadows
      m_UberShader->setBool("softShadows", m_SoftShadows);
      m_UberShader->setInt("shadowMap", 4);

      // Set light uniforms
      m_UberShader->setVec3("viewPos", camera.Position);
      m_UberShader->setVec3("lightPos", m_LightPos);
      m_UberShader->setMat4("lightSpaceMatrix", m_ShadowMap->lightSpaceMatrix);

      // Material
      m_UberShader->setInt("material.diffuseMap", 0);
      m_UberShader->setInt("normalMap", 1);
      m_UberShader->setInt("specularMap", 2);
      m_UberShader->setInt("maskMap", 3);
      m_UberShader->setFloat("material.shininess", params.s);

      // Light
      m_UberShader->setVec3("light.position", m_LightPos);
      m_UberShader->setVec3("light.ambient", glm::vec3(params.la));
      m_UberShader->setVec3("light.diffuse", glm::vec3(params.ld));
      m_UberShader->setVec3("light.specular", glm::vec3(params.ls));
    }

  protected:
    // Timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    
    // Lighting
    glm::vec3 m_LightPos = glm::vec3(3.0f, 6.5f, -14.5f);
    Model* m_Lamp;

    // Shaders
    Shader* m_LampShader;
    Shader* m_UberShader;

    // Matrices
    glm::mat4 m_View;
    glm::mat4 m_Projection;

    // Features
    Godrays* m_Godrays;
    GodraysParams m_GodraysParams;
    ShadowMap* m_ShadowMap;

    // Toggle features
    bool m_GodraysEnabled;
    bool m_Shadows;
    bool m_SoftShadows;

    void DrawLamp()
    {
      m_LampShader->use();
      m_LampShader->setMat4("projection", m_Projection);
      m_LampShader->setMat4("view", m_View);
      glm::mat4 model = glm::mat4();
      model = glm::translate(model, m_LightPos);
      model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
      m_LampShader->setMat4("model", model);

      m_Lamp->Draw(*m_LampShader);
    }

  private:
    void processInput() const
    {
      if (glfwGetKey(s_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(s_Window, true);

      if (glfwGetKey(s_Window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
      if (glfwGetKey(s_Window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
      if (glfwGetKey(s_Window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
      if (glfwGetKey(s_Window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
};

#endif
