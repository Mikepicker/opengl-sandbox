#ifndef STENCIL_SCENE_H
#define STENCIL_SCENE_H

#include "Scene.h"
#include "Shader.h"
#include "Mesh.h"
#include "OBJImporter.h"

#include <iostream>
#include <vector>

class StencilScene : public Scene
{
  public:
    StencilScene(GLFWwindow* window, unsigned int width, unsigned int height)
      : Scene(window, width, height)
    {
      // Light position
      lightPos = glm::vec3(1.2f, 1.0f, 2.0f);

      // configure global opengl state
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_STENCIL_TEST);

      // build and compile our shader zprogram
      lightingShader = new Shader("custom.vs", "custom.fs");
      outlineShader = new Shader("outline.vs", "outline.fs");
      lampShader = new Shader("lamp.vs", "lamp.fs");

      // set up vertex data (and buffer(s)) and configure vertex attributes
      std::vector<Vertex> vertices;
      std::vector<unsigned int> indices;
      OBJImporter importer;
      importer.importOBJ("res/models/militia.obj", vertices, indices);

      // Model mesh
      modelMesh = new Mesh(vertices, indices);
      outlineMesh = new Mesh(vertices, indices);

      // Lamp mesh
      lampMesh = new Mesh(vertices, indices);

      diffuseMap = loadTexture("res/textures/militia.jpg");
      specularMap = loadTexture("res/textures/crate_spec.png");

    }

    void Draw()
    {
      // per-frame time logic
      // --------------------
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      // input
      // -----
      processInput(window);

      // render
      // ------
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      // view/projection transformations
      projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
      view = camera.GetViewMatrix();

      // Draw model
      drawWithOutline(*modelMesh, *outlineMesh, *lightingShader, *outlineShader); 

      // also draw the lamp object
      lampShader->use();
      lampShader->setMat4("projection", projection);
      lampShader->setMat4("view", view);
      glm::mat4 model = glm::mat4();
      model = glm::translate(model, lightPos);
      model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
      lampShader->setMat4("model", model);

      lampMesh->Draw(*lampShader);
    }

    // Sandbox functions
    void drawWithOutline(Mesh& modelMesh, Mesh& outlineMesh, Shader& modelShader, Shader& outlineShader)
      {
    // Set stencil buffer to draw base model
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);

    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);

    // Set shader uniforms
    modelShader.use();
    modelShader.setMat4("projection", projection);
    modelShader.setMat4("view", view);
    modelShader.setInt("material.diffuse", 0);
    modelShader.setInt("material.specular", 1);
    modelShader.setFloat("material.shininess", 64.0f);
    modelShader.setVec3("light.position", lightPos);
    modelShader.setVec3("light.ambient", 1.0f, 1.0f, 1.0f);
    modelShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    modelShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    modelShader.setVec3("viewPos", camera.Position);

    glm::mat4 model;
    modelShader.setMat4("model", model);

    // morph transformation
    glm::mat4 morph;
    //morph = glm::translate(morph, glm::vec3(1.0f, 1.0f, 0.0f));
    morph = glm::rotate(morph, (float)glfwGetTime(), glm::vec3(0, 1.0f, 0));
    //morph = glm::translate(morph, glm::vec3(-1.0f, -1.0f, 0.0f));
    modelShader.setMat4("morph", morph);
    //modelShader.setFloat("mixFactor", sin(glfwGetTime()));
    modelShader.setFloat("mixFactor", 1.0f);

    // render the model
    modelMesh.Draw(modelShader);

    // OUTLINE
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);

    float scale = 1.02f;
    model = glm::mat4();
    model = glm::translate(model, glm::vec3(0, 1-scale, 0));
    model = glm::scale(model, glm::vec3(scale));

    outlineShader.use();
    outlineShader.setMat4("projection", projection);
    outlineShader.setMat4("view", view);
    outlineShader.setMat4("model", model);

    outlineMesh.Draw(outlineShader);

    glStencilMask(0xFF);
    glEnable(GL_DEPTH_TEST);

    }

  private:
    Shader* lightingShader;
    Shader* outlineShader;
    Shader* lampShader;
    Mesh* modelMesh;
    Mesh* outlineMesh;
    Mesh* lampMesh;
    unsigned int diffuseMap;
    unsigned int specularMap;

    // Matrices
    glm::mat4 view;
    glm::mat4 projection;
    
    // lighting
    glm::vec3 lightPos;

};
#endif
