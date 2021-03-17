/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: SimpleScene_Quad.cpp, scene setup
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 15-11-19
*/

#include "SimpleScene_Quad.h"
#include <shader.hpp>
#include <glm/vec3.hpp>
#include <glm/detail/type_mat.hpp>
#include <time.h>

#include "Editor.h"
#include "Light.h"
#include "Deffered.h"
#include "Camera.h"
#include "BVHTree.h"
#include "Node.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/transform.hpp>

void APIENTRY openGLDebug(GLenum source, GLenum type, GLuint id,
  GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

extern Editor* editor;
extern bool FaceNormals;
extern bool VertexNormals;
extern float distFromCenter;

extern glm::mat4 pers;
extern Camera camera;

Deferred* deffer = nullptr;

Object* quad = nullptr;

int numberOfSpheres = 16;
int numberOfSpheresDraw = 1;
glm::vec3 Spheredistance = { 1.0f , 0.0f, 0.0f };

GLuint diffuseTexture = -1;
GLuint specularTexture = -1;
int TextureSize = 512;

std::vector<Light> lights;

vec3 colours[16] =
{
{128, 0, 0},
{170, 110, 40},
{128, 128, 0},
{230, 25, 75},
{245, 130, 48},
{255, 225, 25},
{210, 245, 60},
{60, 180, 75},
{70, 240, 240},
{0, 130, 200},
{145, 30, 180},
{240, 50, 230},
{0, 128, 128},
{0, 0, 128},
{230, 190, 255},
{255, 250, 200}
};

void SimpleScene_Quad::Input()
{
  camera.Input(window);
  camera.Update();
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void SimpleScene_Quad::SetupNanoGUI(GLFWwindow* pWindow)
{
  window = pWindow;
}

void SetUpLights()
{
  //for all lights set default values
  for (int i = 0; i < numberOfSpheres; i++)
  {
    Light light(vec3(1.0f), vec3(1.0f));
    light.LightType = POINTLIGHT;
    light.diffuseColour = vec3(1.0f);
    light.ambientColour = vec3(1.0f);
    light.specularColour = vec3(1.0f);

    lights.push_back(light);
  }

  lights[0].diffuseColour = vec3(0.0f, 1.0f, 1.0f);

  //create UBO
  glGenBuffers(1, &quad->UBO);
  //bind UBO
  glBindBuffer(GL_UNIFORM_BUFFER, quad->UBO);
  //populate UBO
  glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * lights.size(), lights.data(), GL_DYNAMIC_DRAW);
  //unbind UBO
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
SimpleScene_Quad::~SimpleScene_Quad()
{
  initMembers();
  CleanUp();
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
SimpleScene_Quad::SimpleScene_Quad(int windowWidth, int windowHeight) : Scene(windowWidth, windowHeight),
phongShading(0)
{
  srand(size_t(time(nullptr)));

  for (int i = 0; i < numberOfSpheres; i++)
  {
    colours[i] /= 255.0f;
  }

  //create mesh vao's for the following meshes (usually read in)
  MeshManager::Instance()->AddMesh("quad");
  MeshManager::Instance()->AddMesh("sphere");
  MeshManager::Instance()->AddMesh("cube2");

  initMembers();

  deffer = new Deferred(windowWidth, windowHeight);

  //create quad for deferred step
  quad = new Object("quad");
  quad->mesh = MeshManager::Instance()->meshes["quad"];
  quad->transform.position = vec3(-1.0f, -1.0f, 0.0f);
  quad->transform.scale = vec3(2.0f, 2.0f, 0.0f);
  SetUpLights();

  //Mesh* temp = MeshManager::Instance()->LoadMeshAssImp("../Common/models/g4.ply");

  tree = BVHTree::Instance();
  LoadScene("../Common/models/PowerPlant");
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void SimpleScene_Quad::initMembers()
{
  phongShading = 0;
  linesShader = 0;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void SimpleScene_Quad::CleanUp()
{
  // Cleanup VBO
  glDeleteProgram(phongShading);
  glDeleteProgram(linesShader);
  glDeleteProgram(simple);
  glDeleteProgram(deferredQuad);
  glDeleteProgram(deffer->deferedShader);

  glDeleteBuffers(1, &quad->UBO);

  glDeleteTextures(1, &diffuseTexture);
  glDeleteTextures(1, &specularTexture);

  delete deffer;
  delete quad;
  delete tree;

  if (tree->type == BVHType::BottomUP)
  {
    for (size_t i = 0; i < sceneMeshes.size(); i++)
    {
      delete sceneMeshes[i]->node;
    }
  }
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void SimpleScene_Quad::SetupBuffers()
{
  return;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
int SimpleScene_Quad::Init()
{
  // Create and compile our GLSL program from the shaders
  phongShading = LoadShaders("../Common/shaders/PhongShading.vert",
    "../Common/shaders/PhongShading.frag");

  simple = LoadShaders("../Common/shaders/Simple.vert",
    "../Common/shaders/Simple.frag");

  linesShader = LoadShaders("../Common/shaders/QuadVertexShader.vert",
    "../Common/shaders/QuadFragmentShader.frag");

  deferredQuad = LoadShaders("../Common/shaders/DeferredQuad.vert",
    "../Common/shaders/DeferredQuad.frag");

  SetupBuffers();

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);

#ifndef NDEBUG
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(openGLDebug, this);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
  return Scene::Init();
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
int SimpleScene_Quad::Render()
{
  //clear everything
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  editor->UpdateP1();
  //draw deferred objects and populate g-buffer
  deffer->Render(&sceneMeshes);

  //render quad
  RenderQuad();

  //clear depth buffer as the quad write to the depth buffer as well
  glClear(GL_DEPTH_BUFFER_BIT);

  //copy depth values of the deferred objects
  if (deffer->copyDepth)
  {
    deffer->CopyDepthBuffer();
  }

  //draw normals / sphere path
  DrawDebug();

  editor->UpdateP2();

  return 0;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
static float angleRotate = 0.005f;
int SimpleScene_Quad::postRender()
{
  lights[0].position = camera.cam_position;
  lights[0].direction = camera.cam_front;
  glNamedBufferSubData(quad->UBO, 0, sizeof(Light) * lights.size(), lights.data());

  Input();

  return 0;
}

void SimpleScene_Quad::DrawDebug()
{
  if (FaceNormals)
  {
    glUseProgram(linesShader);

    for (size_t i = 0; i < objects.size(); i++)
    {
      objects[i]->DrawFaceNormal(linesShader);
    }

    glUseProgram(0);
  }

  if (VertexNormals)
  {
    glUseProgram(linesShader);

    for (size_t i = 0; i < objects.size(); i++)
    {
      objects[i]->DrawVertexNormal(linesShader);
    }

    glUseProgram(0);
  }

  tree->DrawBVH();
}

void SimpleScene_Quad::RenderQuad()
{
  //use shader for using g-buffer
  glUseProgram(deferredQuad);
  glBindVertexArray(quad->mesh->VAO);

  quad->transform.Update();

  //set uniforms for phong model
  quad->SetUniforms(deferredQuad);

  //send g-buffer render targets as texture to shader
  //position
  GLuint uniform = glGetUniformLocation(deferredQuad, "posTex");
  if (uniform >= 0)
  {
    glUniform1i(uniform, 0);
  }

  //normals
  uniform = glGetUniformLocation(deferredQuad, "normTex");
  if (uniform >= 0)
  {
    glUniform1i(uniform, 1);
  }

  //uv
  uniform = glGetUniformLocation(deferredQuad, "uvTex");
  if (uniform >= 0)
  {
    glUniform1i(uniform, 2);
  }

  //which render target to display
  uniform = glGetUniformLocation(deferredQuad, "target");
  if (uniform >= 0)
  {
    glUniform1i(uniform, deffer->FSQTarget);
  }

  //render quad
  glDrawElements(GL_TRIANGLES, quad->mesh->NumberOfElements, GL_UNSIGNED_INT, 0);
}


#include <experimental/filesystem>
#include <iostream>
namespace fs = std::experimental::filesystem;

void SimpleScene_Quad::LoadScene(const std::string& filepath)
{
  fs::path dir(filepath);
  fs::directory_iterator iterator(dir);

  for (auto& folder : iterator)
  {
    fs::path currFolder = folder.path();

    fs::directory_iterator partIterator(currFolder);

    for (auto& part : partIterator)
    {
      fs::path currPart = part.path();
      fs::directory_iterator modelIterator(currPart);

      for (auto& model : modelIterator)
      {
        std::string modelDir = model.path().string();
        Mesh* current = MeshManager::Instance()->LoadMeshAssImp(modelDir);

        if (current)
        {
          sceneMeshes.push_back(current);
          meshPool.insert(meshPool.end(), current->vertex.begin(), current->vertex.end());
          meshNormals.insert(meshNormals.end(), current->vertexNormals.begin(), current->vertexNormals.end());
        }
      }
    }
  }

  tree->BuildTreeBottomUp(sceneMeshes);
}