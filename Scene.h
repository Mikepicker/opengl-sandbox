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

    virtual void Draw() {}

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

    // Generate and bind FBO
    void generateFBORBO(unsigned int &framebuffer, unsigned int &texColorBuffer, unsigned int &rbo)
    {
      glGenFramebuffers(1, &framebuffer);
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

      // generate texture
      glGenTextures(1, &texColorBuffer);
      glBindTexture(GL_TEXTURE_2D, texColorBuffer);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s_WindowWidth/4, s_WindowHeight/4, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glBindTexture(GL_TEXTURE_2D, 0);

      // attach it to currently bound framebuffer object
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

      // Generate and bind render buffer for depth and stencil tests
      glGenRenderbuffers(1, &rbo);
      glBindRenderbuffer(GL_RENDERBUFFER, rbo);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      // Attach rbo to fbo
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

      // Check attachments and unbind fbo
      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Generate quad for post processing
    void generateQuad(unsigned int &quadVAO, unsigned int &quadVBO)
    {
      float quadVertices[] = {
        -1.0f, 1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f,  0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f,  1.0f, 1.0f
      };
      glGenVertexArrays(1, &quadVAO);
      glGenBuffers(1, &quadVBO);
      glBindVertexArray(quadVAO);
      glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    }

  protected:

    // timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

};

#endif
