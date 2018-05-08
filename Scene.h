#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "dep/glm/glm.hpp"
#include "dep/glm/gtc/matrix_transform.hpp"
#include "dep/glm/gtc/type_ptr.hpp"
#include "dep/stb_image/stb_image.h"

#include "Camera.h"

// Static variables for GLFW
float lastX = 0.0f;
float lastY = 0.0f;
bool firstMouse = true;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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
  camera.ProcessMouseScroll(yoffset);
}

class Scene
{
  public:
    Scene(GLFWwindow* window, unsigned int width, unsigned int height) : window(window), windowWidth(width), windowHeight(height)
    {
      float lastX = width / 2.0f;
      float lastY = height / 2.0f;

      glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
      glfwSetCursorPosCallback(window, mouse_callback);
      glfwSetScrollCallback(window, scroll_callback);

    }

    //virtual void Draw();
    void processInput(GLFWwindow *window)
    {
      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

      if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
      if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
      if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
      if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
    
    // utility function for loading a 2D texture from file
    // ---------------------------------------------------
    unsigned int loadTexture(char const * path)
    {
      unsigned int textureID;
      glGenTextures(1, &textureID);

      int width, height, nrComponents;
      unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
      if (data)
      {
        GLenum format;
        if (nrComponents == 1)
          format = GL_RED;
        else if (nrComponents == 3)
          format = GL_RGB;
        else if (nrComponents == 4)
          format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
      }
      else
      {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
      }

      return textureID;
    }

  protected:
    // window
    GLFWwindow* window;
    unsigned int windowWidth;
    unsigned int windowHeight;

    // timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

};

#endif
