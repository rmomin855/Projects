//
// Created by pushpak on 6/1/18.
//

#ifndef SIMPLE_SCENE_SIMPLESCENE_QUAD_H
#define SIMPLE_SCENE_SIMPLESCENE_QUAD_H

#include "../Common/Scene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

class SimpleScene_Quad : public Scene
{

public:
  SimpleScene_Quad() = default;
  SimpleScene_Quad(int windowWidth, int windowHeight);
  virtual ~SimpleScene_Quad();


public:
  int Init() override;
  void CleanUp() override;

  int Render() override;
  int postRender() override;

  int preRender() override;


  // data members
  GLuint phongShading;
  GLuint phongLighting;
  GLuint blinnShading;
  GLuint linesShader;
  GLuint simple;
  GLuint skyBox;

  bool withEditor = true;

private:

  // member functions
  void initMembers();

  // This is the non-software engineered portion of the code
  // Go ahead and modularize the VAO and VBO setup into
  // BufferManagers and ShaderManagers
  void SetupBuffers();

  void SetupNanoGUI(GLFWwindow* pWwindow) override;

  void DrawDebug();
};

void UpdateSphereRing();

#endif //SIMPLE_SCENE_SIMPLESCENE_QUAD_H
