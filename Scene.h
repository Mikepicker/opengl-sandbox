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
    }

    virtual void Draw() {
      // Per-frame time logic
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      // Input
      processInput();
    }

    void processInput()
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

    // timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

};

#endif
