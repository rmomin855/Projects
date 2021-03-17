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

Object* ring = nullptr;
Object* centralObject = nullptr;
Object* flatObject = nullptr;

int numberOfSpheres = 16;
int numberOfSpheresDraw = 16;
glm::vec3 Spheredistance = { 1.0f , 0.4, 0.0f };

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

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void SimpleScene_Quad::SetupNanoGUI(GLFWwindow* pWindow)
{
  pWindow = nullptr;
}

void SetUpLights()
{
  for (int i = 0; i < numberOfSpheres; i++)
  {
    Light light(spheres[i]->transform.position, centralObject->transform.position - spheres[i]->transform.position);
    light.LightType = POINTLIGHT;
    light.diffuseColour = spheres[i]->colour;
    light.ambientColour = centralObject->colour;
    light.specularColour = vec3(1.0f);

    lights.push_back(light);
  }

  glGenBuffers(1, &centralObject->UBO);
  glBindBuffer(GL_UNIFORM_BUFFER, centralObject->UBO);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * lights.size(), lights.data(), GL_DYNAMIC_DRAW);
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
phongShading(0), phongLighting(0), blinnShading(0)
{
  for (int i = 0; i < numberOfSpheres; i++)
  {
    colours[i] /= 255.0f;
  }

  Spheredistance = normalize(Spheredistance);

  MeshManager::Instance()->AddMesh("quad");
  MeshManager::Instance()->AddMesh("sphere", true);

  centralObject = new Object("CentralObject");
  centralObject->mesh = MeshManager::Instance()->meshes["sphere"];
  centralObject->transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
  centralObject->transform.scale = glm::vec3(0.3f, 0.3f, 0.3f);
  centralObject->drawType = DrawType::Fill;
  centralObject->colour = glm::vec3(1.0f, 1.0f, 1.0f);

  objects.push_back(centralObject);

  glm::vec3 distance = Spheredistance;
  distance = normalize(distance);
  constexpr float angleStep = glm::radians(360.0f / 16.0f);
  float angle = 0.0f;

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

  MeshManager::Instance()->SetUpRing();

  ring = new Object("ring");
  ring->transform.position = centralObject->transform.position;
  ring->transform.scale = vec3(1.0f);
  ring->mesh = MeshManager::Instance()->meshes["ring"];
  ring->transform.Update();

  ring->transform.transformMatrix = view * ring->transform.transformMatrix;
  initMembers();

  SetUpLights();

  flatObject = new Object("flat");
  flatObject->mesh = MeshManager::Instance()->meshes["quad"];
  flatObject->transform.position = glm::vec3(0.0f, -0.3f, 0.0f);
  flatObject->transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
  flatObject->transform.rotation = glm::vec3(radians(-90.0f), 0.0f, 0.0f);
  flatObject->drawType = DrawType::Fill;
  flatObject->colour = glm::vec3(1.0f, 1.0f, 1.0f);
  flatObject->texture = false;

  objects.push_back(flatObject);

  diffuseTexture = MeshManager::Instance()->LoadTexture("metal_roof_diff_512x512");
  specularTexture = MeshManager::Instance()->LoadTexture("metal_roof_spec_512x512");

  glActiveTexture(GL_TEXTURE5);
  glActiveTexture(GL_TEXTURE6);

  glBindTextureUnit(DIFFUSE, diffuseTexture);
  glBindTextureUnit(SPECULAR, specularTexture);

  glTextureParameteri(diffuseTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTextureParameteri(diffuseTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTextureParameteri(diffuseTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(diffuseTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTextureParameteri(specularTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTextureParameteri(specularTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTextureParameteri(specularTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(specularTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-unused-return-value"
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void SimpleScene_Quad::initMembers()
{
  phongShading = 0;
  linesShader = 0;
  phongLighting = 0;
  blinnShading = 0;
}
#pragma clang diagnostic pop

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void SimpleScene_Quad::CleanUp()
{
  // Cleanup VBO
  glDeleteProgram(phongShading);
  glDeleteProgram(phongLighting);
  glDeleteProgram(linesShader);
  glDeleteProgram(blinnShading);

  glDeleteBuffers(1, &centralObject->UBO);

  glDeleteTextures(1, &diffuseTexture);
  glDeleteTextures(1, &specularTexture);
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

  phongLighting = LoadShaders("../Common/shaders/PhongLighting.vert",
    "../Common/shaders/PhongLighting.frag");

  linesShader = LoadShaders("../Common/shaders/QuadVertexShader.vert",
    "../Common/shaders/QuadFragmentShader.frag");

  blinnShading = LoadShaders("../Common/shaders/BlinnShading.vert",
    "../Common/shaders/BlinnShading.frag");

  simple = LoadShaders("../Common/shaders/Simple.vert",
    "../Common/shaders/Simple.frag");

  centralObject->shaderToUse = Shaders::phongShading;
  flatObject->shaderToUse = Shaders::phongShading;

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
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  editor->UpdateP1();


  for (size_t i = 0; i < objects.size(); i++)
  {
    GLuint program = 0;

    switch (objects[i]->shaderToUse)
    {
    case Shaders::phongShading:
      program = phongShading;
      break;
    case Shaders::blinnShading:
      program = blinnShading;
      break;
    case Shaders::phongLighting:
      program = phongLighting;
      break;
    case Shaders::simple:
      program = simple;
      break;
    default:
      break;
    }
    objects[i]->Update();
    objects[i]->Draw(program);
  }


  for (int i = 0; i < numberOfSpheresDraw; i++)
  {
    GLuint program = 0;

    switch (spheres[i]->shaderToUse)
    {
    case Shaders::phongShading:
      program = phongShading;
      break;
    case Shaders::blinnShading:
      program = blinnShading;
      break;
    case Shaders::phongLighting:
      program = phongLighting;
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
    for (size_t i = 0; i < spheres.size(); i++)
    {
      spheres[i]->transform.position = glm::rotateY(spheres[i]->transform.position, angleRotate);
    }
  }

  for (int i = 0; i < numberOfSpheres; i++)
  {
    lights[i].position = vec3(spheres[i]->transform.transformMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f));
    lights[i].direction = vec3(centralObject->transform.transformMatrix * vec4(centralObject->transform.position, 1.0f)) - lights[i].position;
  }

  glNamedBufferSubData(centralObject->UBO, 0, sizeof(Light) * lights.size(), lights.data());

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

void UpdateSphereRing()
{
  vec3 distVec = spheres[0]->transform.position - centralObject->transform.position;

  float angle = 360.0f / numberOfSpheresDraw;

  for (size_t i = 1; i < numberOfSpheresDraw; i++)
  {
    vec3 newDist = rotateY(distVec, radians(i * angle));

    spheres[i]->transform.position = centralObject->transform.position + newDist;
  }

  for (float i = 0.0f; i < 360.0f; i++)
  {
    Ringvertices[size_t(i)] = glm::rotateY(Spheredistance, glm::radians(i));
  }

  glNamedBufferSubData(ring->mesh->VertexVBO, 0,  // same as glBufferSubData
    sizeof(glm::vec3) * Ringvertices.size(), Ringvertices.data());
}