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

#include "Editor.h"
#include "Light.h"
#include "Deffered.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/transform.hpp>

void APIENTRY openGLDebug(GLenum source, GLenum type, GLuint id,
  GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

extern Editor* editor;
extern bool FaceNormals;
extern bool VertexNormals;
extern bool DrawRing;
extern bool rotateLights;
extern float distFromCenter;

extern glm::mat4 pers;
extern glm::mat4 view;
extern glm::vec3 cameraPos;
extern glm::vec3 cameraLook;

Object* ring = nullptr;
Object* centralObject = nullptr;

Deferred* deffer = nullptr;

Object* quad = nullptr;

int numberOfSpheres = 16;
int numberOfSpheresDraw = 16;
glm::vec3 Spheredistance = { 1.0f , 0.0f, 0.0f };

GLuint diffuseTexture = -1;
GLuint specularTexture = -1;
int TextureSize = 512;

std::vector<Object*> spheres;
std::vector<glm::vec3> Ringvertices;
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
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  {
    cameraPos += vec3(0.0f, 0.0f, -0.05f);
    cameraLook += vec3(0.0f, 0.0f, -0.05f);
    view = glm::lookAt(cameraPos, cameraLook, glm::vec3(0.0f, 1.0f, 0.0f));
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
  {
    cameraPos -= vec3(0.0f, 0.0f, -0.05f);
    cameraLook -= vec3(0.0f, 0.0f, -0.05f);
    view = glm::lookAt(cameraPos, cameraLook, glm::vec3(0.0f, 1.0f, 0.0f));
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
  {
    cameraPos -= vec3(0.05f, 0.0f, 0.0f);
    cameraLook -= vec3(0.05f, 0.0f, 0.0f);
    view = glm::lookAt(cameraPos, cameraLook, glm::vec3(0.0f, 1.0f, 0.0f));
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
  {
    cameraPos += vec3(0.05f, 0.0f, 0.0f);
    cameraLook += vec3(0.05f, 0.0f, 0.0f);
    view = glm::lookAt(cameraPos, cameraLook, glm::vec3(0.0f, 1.0f, 0.0f));
  }
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
    Light light(spheres[i]->transform.position, centralObject->transform.position - spheres[i]->transform.position);
    light.LightType = POINTLIGHT;
    light.diffuseColour = spheres[i]->colour;
    light.ambientColour = centralObject->colour;
    light.specularColour = vec3(1.0f);

    lights.push_back(light);
  }

  //create UBO
  glGenBuffers(1, &centralObject->UBO);
  //bind UBO
  glBindBuffer(GL_UNIFORM_BUFFER, centralObject->UBO);
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
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
SimpleScene_Quad::SimpleScene_Quad(int windowWidth, int windowHeight) : Scene(windowWidth, windowHeight),
phongShading(0)
{
  for (int i = 0; i < numberOfSpheres; i++)
  {
    colours[i] /= 255.0f;
  }

  //create mesh vao's for the following meshes (usually read in)
  MeshManager::Instance()->AddMesh("quad");
  MeshManager::Instance()->AddMesh("sphere", true);

  centralObject = new Object("CentralObject");
  centralObject->mesh = MeshManager::Instance()->meshes["sphere"];
  centralObject->transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
  centralObject->transform.scale = glm::vec3(0.5f, 0.5f, 0.5f);
  centralObject->drawType = DrawType::Fill;
  centralObject->colour = glm::vec3(1.0f, 1.0f, 1.0f);

  objects.push_back(centralObject);

  glm::vec3 distance = normalize(Spheredistance);
  distance *= distFromCenter;

  constexpr float angleStep = glm::radians(360.0f / 16.0f);
  float angle = 0.0f;

  //for 16 spheres, create object and initialize values
  for (int i = 0; i < numberOfSpheres; i++)
  {
    Object* sphere = new Object("sphere");
    sphere->mesh = MeshManager::Instance()->meshes["sphereGen"];

    glm::vec3 currDist = glm::rotateY(distance, angle);
    angle += angleStep;

    sphere->transform.position = centralObject->transform.position + currDist;
    sphere->transform.scale = glm::vec3(0.05f, 0.05f, 0.05f);
    sphere->drawType = DrawType::Fill;
    sphere->colour = colours[i];
    sphere->shaderToUse = Shaders::simple;

    spheres.push_back(sphere);
  }

  //calculate mesh data for ring
  MeshManager::Instance()->SetUpRing();

  //create sphere ring object
  ring = new Object("ring");
  ring->transform.position = centralObject->transform.position;
  ring->transform.scale = vec3(1.0f);
  ring->mesh = MeshManager::Instance()->meshes["ring"];
  ring->transform.Update();

  ring->transform.transformMatrix = view * ring->transform.transformMatrix;

  initMembers();

  SetUpLights();

  //load textures and create buffers
  diffuseTexture = MeshManager::Instance()->LoadTexture("metal_roof_diff_512x512");
  specularTexture = MeshManager::Instance()->LoadTexture("metal_roof_spec_512x512");

  //activate textures for phong model
  glActiveTexture(GL_TEXTURE5);
  glActiveTexture(GL_TEXTURE6);

  //bind to texture locations
  glBindTextureUnit(DIFFUSE, diffuseTexture);
  glBindTextureUnit(SPECULAR, specularTexture);

  deffer = new Deferred(windowWidth, windowHeight);

  //create quad for deferred step
  quad = new Object("quad");
  quad->mesh = MeshManager::Instance()->meshes["quad"];
  quad->transform.position = vec3(-1.0f, -1.0f, 0.0f);
  quad->transform.scale = vec3(2.0f, 2.0f, 0.0f);
  quad->UBO = centralObject->UBO;

  UpdateSphereRing();
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

  glDeleteBuffers(1, &centralObject->UBO);

  glDeleteTextures(1, &diffuseTexture);
  glDeleteTextures(1, &specularTexture);

  delete deffer;
  delete ring;
  delete centralObject;
  delete quad;

  for (size_t i = 0; i < numberOfSpheres; i++)
  {
    delete spheres[i];
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

  centralObject->shaderToUse = Shaders::phongShading;

  for (size_t i = 0; i < 16; i++)
  {
    spheres[i]->shaderToUse = Shaders::simple;
  }

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
  deffer->Render(objects);

  //render quad
  RenderQuad();

  //clear depth buffer as the quad write to the depth buffer as well
  glClear(GL_DEPTH_BUFFER_BIT);

  //copy depth values of the deferred objects
  if (deffer->copyDepth)
  {
    deffer->CopyDepthBuffer();
  }

  //forward rendering of light spheres
  for (int i = 0; i < numberOfSpheresDraw; i++)
  {
    GLuint program = 0;

    switch (spheres[i]->shaderToUse)
    {
    case Shaders::phongShading:
      program = phongShading;
      break;
    case Shaders::simple:
      program = simple;
      break;
    default:
      break;
    }
    spheres[i]->Update();
    spheres[i]->Draw(program);
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
  if (rotateLights)
  {
    //rotate each sphere
    for (size_t i = 0; i < spheres.size(); i++)
    {
      spheres[i]->transform.position = glm::rotateY(spheres[i]->transform.position, angleRotate);
    }

    //update corresponding light information
    for (int i = 0; i < numberOfSpheres; i++)
    {
      lights[i].position = vec3(spheres[i]->transform.transformMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f));
      lights[i].direction = vec3(centralObject->transform.transformMatrix * vec4(centralObject->transform.position, 1.0f)) - lights[i].position;
    }

    //update ubo buffer
    glNamedBufferSubData(centralObject->UBO, 0, sizeof(Light) * lights.size(), lights.data());
  }

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

  if (DrawRing)
  {
    glUseProgram(linesShader);
    ring->DrawVertexNormal(linesShader);
    glUseProgram(0);
  }
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

void UpdateSphereRing()
{
  //get distance vector
  vec3 distVec = spheres[0]->transform.position - centralObject->transform.position;

  //know angle offset
  float angle = 360.0f / numberOfSpheresDraw;

  //for each sphere being drawn
  for (size_t i = 1; i < numberOfSpheresDraw; i++)
  {
    //rotate vector
    vec3 newDist = rotateY(distVec, radians(i * angle));

    //update sphere location
    spheres[i]->transform.position = centralObject->transform.position + newDist;
  }

  //re-calculate ring vertices
  for (float i = 0.0f; i < 360.0f; i++)
  {
    Ringvertices[size_t(i)] = glm::rotateY(distVec, glm::radians(i));
  }

  //update buffer
  glNamedBufferSubData(ring->mesh->VertexVBO, 0,  // same as glBufferSubData
    sizeof(glm::vec3) * Ringvertices.size(), Ringvertices.data());
}