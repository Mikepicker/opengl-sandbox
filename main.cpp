#include <string>
#include <vector>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "StencilScene.h"
#include "FBOScene.h"
#include "GodraysScene.h"
#include "TerrainScene.h"
#include "NormalMapScene.h"
#include "CryptScene.h"

#define print(s) std::cout << s << std::endl;

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // glfw window creation
  // --------------------
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  //StencilScene scene(window, SCR_WIDTH, SCR_HEIGHT);
  //FBOScene scene(window, SCR_WIDTH, SCR_HEIGHT);
  //GodraysScene scene(window, SCR_WIDTH, SCR_HEIGHT);
  //TerrainScene scene(window, SCR_WIDTH, SCR_HEIGHT);
  //NormalMapScene scene(window, SCR_WIDTH, SCR_HEIGHT);
  CryptScene scene(window, SCR_WIDTH, SCR_HEIGHT);

  std::cout << glfwGetVersionString() << std::endl;

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window))
  {
    scene.Draw();  
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwTerminate();
  return 0; 
}


