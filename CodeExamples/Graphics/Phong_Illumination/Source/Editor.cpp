/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Editor.cpp, handle all imgui calls for buttons and checks
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 3-10-19
*/

#include "Editor.h"
#include "MeshManager.h"
#include <Scene.h>
#include "SimpleScene_Quad.h"
#include <shader.hpp>

// Include GLFW
#include <GLFW/glfw3.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "Light.h"

#include <experimental/filesystem>
#include <filesystem>

void UpdateUVBuffer();

Editor* Editor::editor = nullptr;
bool FaceNormals = false;
bool VertexNormals = false;
bool DrawRing = false;
bool rotateLights = true;

bool CPUUV = false;

extern Object* centralObject;
extern std::vector<Object*> spheres;
extern std::vector<Light> lights;
extern vec3 colours[16];

extern MeshManager* meshManager;
extern GLFWwindow* window;
extern Scene* scene;

extern int numberOfSpheresDraw;
extern int numberOfSpheres;
extern glm::vec3 Spheredistance;

extern float near;
extern float far;

extern mat4 pers;

char input[1000] = {};

float lightAngle = 13.0f;
float innerAngle = 0.0f;
float attunation[3] = { 1.0f, 0.5f, 0.25f };
float emissive[3] = { 10.0f, 10.0f, 10.0f };
float fog[3] = { 80.0f, 80.0f, 80.0f };
float ambient[3] = { 255.0f, 255.0f, 255.0f };
float specular[3] = { 255.0f, 255.0f, 255.0f };

float globalAmbientStrength = 0.01f;
float globalDiffuseStrength = 0.5f;
float globalspecularStrength = 0.8f;
float globalShininess = 64.0f;
float distFromCenter = 1.0f;
float falloff = 1.0f;

int lightSelected = 0;
int textureType = 0;

#define CYLIN 0
#define SPHE 1
#define PLAN 2

#define POS 0
#define NORM 1

extern std::vector<glm::vec2> UVCylindricalPos;
extern std::vector<glm::vec2> UVCylindricalNorm;

extern std::vector<glm::vec2> UVSphericalPos;
extern std::vector<glm::vec2> UVSphericalNorm;

extern std::vector<glm::vec2> UVPlanarPos;
extern std::vector<glm::vec2> UVPlanarNorm;
extern int currentMapping;
extern int currentEntity;

Editor::Editor()
{
  const char* version = "#version 450";
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(version);
  ImGui::StyleColorsDark();
}

Editor::~Editor()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Editor::UpdateP1()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void Editor::UpdateP2()
{
  //get screen size
  int x, y;
  glfwGetFramebufferSize(window, &x, &y);
  ImVec2 screenSize = ImVec2(float(x), float(y));

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::Begin("Editor");

  ImGui::BeginTabBar("Menu");

  if (ImGui::BeginTabItem("General"))
  {
    MenuEditor(screenSize);
    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem("Lights/Shaders"))
  {
    LightsEditor(screenSize);
    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem("Per Light"))
  {
    PerLightMenu(screenSize);
    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem("Textures"))
  {
    TextureMenu(screenSize);
    ImGui::EndTabItem();
  }

  ImGui::EndTabBar();

  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::MenuEditor(const ImVec2& screenSize)
{
  ImGui::Checkbox("Draw Face Normals ", &FaceNormals);
  ImGui::Checkbox("Draw Vertex Normals ", &VertexNormals);
  ImGui::Checkbox("Draw Sphere Path ", &DrawRing);

  if (ImGui::Button("Draw Type: Fill"))
  {
    centralObject->drawType = DrawType::Fill;

    int size = spheres.size();
    for (int i = 0; i < size; i++)
    {
      spheres[i]->drawType = DrawType::Fill;
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("Draw Type: Line"))
  {
    centralObject->drawType = DrawType::Line;

    int size = spheres.size();
    for (int i = 0; i < size; i++)
    {
      spheres[i]->drawType = DrawType::Line;
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("Draw Type: Point"))
  {
    centralObject->drawType = DrawType::Point;

    int size = spheres.size();
    for (int i = 0; i < size; i++)
    {
      spheres[i]->drawType = DrawType::Point;
    }
  }

  if (ImGui::Button("Pause/Resume Light Rotation"))
  {
    rotateLights = !rotateLights;
  }

  if (ImGui::Button("Pause/Resume Central Obj Rotation"))
  {
    centralObject->rotateObject = !centralObject->rotateObject;
  }

  float old = near;
  ImGui::InputFloat("Camera near", &near, 1.0f);

  if (old != near)
  {
    pers = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, near, far);
  }

  old = far;
  ImGui::InputFloat("Camera far", &far, 1.0f);

  if (old != far)
  {
    pers = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, near, far);
  }
}

void Editor::ReloadShaders()
{
  SimpleScene_Quad* curr = reinterpret_cast<SimpleScene_Quad*>(scene);
  glDeleteProgram(curr->phongShading);
  glDeleteProgram(curr->phongLighting);
  glDeleteProgram(curr->linesShader);
  glDeleteProgram(curr->blinnShading);
  glDeleteProgram(curr->simple);

  curr->phongShading = LoadShaders("../Common/shaders/PhongShading.vert",
    "../Common/shaders/PhongShading.frag");

  curr->phongLighting = LoadShaders("../Common/shaders/PhongLighting.vert",
    "../Common/shaders/PhongLighting.frag");

  curr->linesShader = LoadShaders("../Common/shaders/QuadVertexShader.vert",
    "../Common/shaders/QuadFragmentShader.frag");

  curr->blinnShading = LoadShaders("../Common/shaders/BlinnShading.vert",
    "../Common/shaders/BlinnShading.frag");

  curr->simple = LoadShaders("../Common/shaders/Simple.vert",
    "../Common/shaders/Simple.frag");
}

void Editor::LightsEditor(const ImVec2& screenSize)
{
  if (ImGui::Button("Reload Shaders"))
  {
    ReloadShaders();
  }

  if (ImGui::Button("Phong Shading"))
  {
    centralObject->shaderToUse = Shaders::phongShading;
  }

  ImGui::SameLine();

  if (ImGui::Button("Phong Lighting"))
  {
    centralObject->shaderToUse = Shaders::phongLighting;
  }

  ImGui::SameLine();

  if (ImGui::Button("Blinn Shading"))
  {
    centralObject->shaderToUse = Shaders::blinnShading;
  }

  int oldnum = numberOfSpheresDraw;
  ImGui::Text("Maximum of 16 lights");
  ImGui::InputInt("Number Of Lights", &numberOfSpheresDraw);

  if (oldnum != numberOfSpheresDraw)
  {
    numberOfSpheresDraw = numberOfSpheresDraw > 16 ? 16 : numberOfSpheresDraw;
    numberOfSpheresDraw = numberOfSpheresDraw < 1 ? 1 : numberOfSpheresDraw;

    UpdateSphereRing();
  }

  float oldlightAngle = lightAngle;
  ImGui::Text("Between 1.0f and 45.0f (outer-half angle)");
  ImGui::InputFloat("Field of light (spot lights)", &lightAngle, 0.5f);

  if (oldlightAngle != lightAngle)
  {
    lightAngle = lightAngle > 45.0f ? 45.0f : lightAngle;
    lightAngle = lightAngle < 1.0f ? 1.0f : lightAngle;

    for (int i = 0; i < numberOfSpheres; i++)
    {
      lights[i].halfFieldOfView = lightAngle;
    }
  }

  float oldInner = innerAngle;
  ImGui::Text("Between 0.0f and Field of light");
  ImGui::InputFloat("Inner Angle (spot lights)", &innerAngle, 0.5f);

  if (oldInner != innerAngle)
  {
    innerAngle = innerAngle >= lightAngle ? lightAngle - 0.1f : innerAngle;
    innerAngle = innerAngle < 0.0f ? 0.0f : innerAngle;

    for (int i = 0; i < numberOfSpheres; i++)
    {
      lights[i].innerAngle = innerAngle;
    }
  }

  float old = falloff;
  ImGui::InputFloat("Fall Off (Spot Light)", &falloff, 0.1f);

  if (old != falloff)
  {
    falloff = falloff < 0.0f ? 0.0f : falloff;

    for (size_t i = 0; i < numberOfSpheres; i++)
    {
      lights[i].fallOff = falloff;
    }
  }

  if (ImGui::Button("Point Lights (all lights)"))
  {
    for (int i = 0; i < numberOfSpheres; i++)
    {
      lights[i].LightType = POINTLIGHT;
    }
  }

  if (ImGui::Button("Directional Lights (all lights)"))
  {
    for (int i = 0; i < numberOfSpheres; i++)
    {
      lights[i].LightType = DIRECTIONALLIGHT;
    }
  }

  if (ImGui::Button("Spot Lights (all lights)"))
  {
    for (int i = 0; i < numberOfSpheres; i++)
    {
      lights[i].LightType = SPOTLIGHT;
    }
  }

  ImGui::InputFloat3("Attunation constants", attunation, 5);

  ImGui::Text("Between 0.0f and 255.0f");
  ImGui::InputFloat3("emissive colour", emissive, 5);

  for (int i = 0; i < 3; i++)
  {
    emissive[i] = emissive[i] > 255.0f ? 255.0f : emissive[i];
    emissive[i] = emissive[i] < 0.0f ? 0.0f : emissive[i];
  }

  ImGui::InputFloat3("fog colour", fog, 5);

  for (int i = 0; i < 3; i++)
  {
    fog[i] = fog[i] > 255.0f ? 255.0f : fog[i];
    fog[i] = fog[i] < 0.0f ? 0.0f : fog[i];
  }

  ImGui::Text("Colours (All Lights) (Between 0.0f and 255.0f)");

  ImGui::InputFloat3("ambient colour", ambient, 5);

  for (int i = 0; i < 3; i++)
  {
    ambient[i] = ambient[i] > 255.0f ? 255.0f : ambient[i];
    ambient[i] = ambient[i] < 0.0f ? 0.0f : ambient[i];
  }

  for (int i = 0; i < numberOfSpheres; i++)
  {
    lights[i].ambientColour = vec3(ambient[0], ambient[1], ambient[2]) / 255.0f;
  }

  ImGui::InputFloat3("specular colour", specular, 5);

  for (int i = 0; i < 3; i++)
  {
    specular[i] = specular[i] > 255.0f ? 255.0f : specular[i];
    specular[i] = specular[i] < 0.0f ? 0.0f : specular[i];
  }

  for (int i = 0; i < numberOfSpheres; i++)
  {
    lights[i].specularColour = vec3(specular[0], specular[1], specular[2]) / 255.0f;
  }


  ImGui::Text("Strengths (All Lights)");

  old = globalAmbientStrength;
  ImGui::InputFloat("Ambient", &globalAmbientStrength, 0.01f);

  if (old != globalAmbientStrength)
  {
    for (int i = 0; i < numberOfSpheres; i++)
    {
      lights[i].ambientStrength = globalAmbientStrength;
    }
  }

  old = globalDiffuseStrength;
  ImGui::InputFloat("Diffuse", &globalDiffuseStrength, 0.01f);

  if (old != globalDiffuseStrength)
  {
    for (int i = 0; i < numberOfSpheres; i++)
    {
      lights[i].diffuseStrength = globalDiffuseStrength;
    }
  }

  old = globalspecularStrength;
  ImGui::InputFloat("Specular", &globalspecularStrength, 0.01f);

  if (old != globalspecularStrength)
  {
    for (int i = 0; i < numberOfSpheres; i++)
    {
      lights[i].specularStrength = globalspecularStrength;
    }
  }

  old = globalShininess;
  ImGui::InputFloat("Shininess", &globalShininess, 1.0f);

  if (old != globalShininess)
  {
    for (int i = 0; i < numberOfSpheres; i++)
    {
      lights[i].shininess = globalShininess;
    }
  }


  old = distFromCenter;
  ImGui::Text("From Central Object (all lights)");
  ImGui::InputFloat("Distance", &distFromCenter, 0.01f);

  if (old != distFromCenter)
  {
    for (int i = 0; i < numberOfSpheres; i++)
    {
      vec3 distVec = spheres[i]->transform.position - centralObject->transform.position;
      distVec = normalize(distVec);

      distVec *= distFromCenter;

      spheres[i]->transform.position = centralObject->transform.position + distVec;

    }

    Spheredistance = normalize(Spheredistance) * distFromCenter;

    UpdateSphereRing();
  }


  if (ImGui::Button("Scene1 (Same colour/type)"))
  {
    int index = int(float(rand()) / float(RAND_MAX) * 16.0f);

    int lightType = int(float(rand()) / float(RAND_MAX) * 3.0f);

    for (int i = 0; i < numberOfSpheres; i++)
    {
      spheres[i]->colour = colours[index];
      lights[i].diffuseColour = colours[index];

      lights[i].LightType = lightType;
    }
  }

  if (ImGui::Button("Scene2 (same colour, different type)"))
  {
    int colour = int(float(rand()) / float(RAND_MAX) * 16.0f);

    for (int i = 0; i < numberOfSpheres; i++)
    {
      spheres[i]->colour = colours[colour];
      lights[i].diffuseColour = colours[colour];

      int type = int(float(rand()) / float(RAND_MAX) * 3.0f);
      lights[i].LightType = type;
    }
  }

  if (ImGui::Button("Scene2 (different colour, same type)"))
  {
    int type = int(float(rand()) / float(RAND_MAX) * 3.0f);

    for (int i = 0; i < numberOfSpheres; i++)
    {
      int colour = int(float(rand()) / float(RAND_MAX) * 16.0f);

      spheres[i]->colour = colours[colour];
      lights[i].diffuseColour = colours[colour];

      lights[i].LightType = type;
    }
  }

  if (ImGui::Button("Scene3 (Mixed)"))
  {
    for (int i = 0; i < numberOfSpheres; i++)
    {
      int index = int(float(rand()) / float(RAND_MAX) * 16.0f);
      spheres[i]->colour = colours[index];
      lights[i].diffuseColour = colours[index];

      int type = int(float(rand()) / float(RAND_MAX) * 3.0f);
      lights[i].LightType = type;
    }
  }
}

void Editor::PerLightMenu(const ImVec2& screenSize)
{
  if (lightSelected >= numberOfSpheresDraw)
  {
    lightSelected = numberOfSpheresDraw - 1;
  }

  int old = lightSelected;
  ImGui::InputInt("Light Index (< active lights)", &lightSelected);

  if (old != lightSelected)
  {
    if (lightSelected >= numberOfSpheresDraw)
    {
      lightSelected = numberOfSpheresDraw - 1;
    }

    if (lightSelected < 0)
    {
      lightSelected = 0;
    }
  }

  Light* selectedLight = &lights[lightSelected];

  ImGui::Text("Between 0.0f and 1.0f");

  float colour[3] = { selectedLight->diffuseColour.r, selectedLight->diffuseColour.g, selectedLight->diffuseColour.b };
  ImGui::InputFloat3("diffuse Colour", colour);
  selectedLight->diffuseColour.r = colour[0];
  selectedLight->diffuseColour.g = colour[1];
  selectedLight->diffuseColour.b = colour[2];

  spheres[lightSelected]->colour.r = colour[0];
  spheres[lightSelected]->colour.g = colour[1];
  spheres[lightSelected]->colour.b = colour[2];

  float ambientcolour[3] = { selectedLight->ambientColour.r, selectedLight->ambientColour.g, selectedLight->ambientColour.b };
  ImGui::InputFloat3("ambient Colour", ambientcolour);
  selectedLight->ambientColour.r = ambientcolour[0];
  selectedLight->ambientColour.g = ambientcolour[1];
  selectedLight->ambientColour.b = ambientcolour[2];

  float specularColour[3] = { selectedLight->specularColour.r, selectedLight->specularColour.g, selectedLight->specularColour.b };
  ImGui::InputFloat3("specular Colour", specularColour);
  selectedLight->specularColour.r = specularColour[0];
  selectedLight->specularColour.g = specularColour[1];
  selectedLight->specularColour.b = specularColour[2];

  ImGui::InputFloat("Ambient Strength", &selectedLight->ambientStrength, 0.01f);
  ImGui::InputFloat("Diffuse Strength", &selectedLight->diffuseStrength, 0.01f);
  ImGui::InputFloat("Specular Strength", &selectedLight->specularStrength, 0.01f);
  ImGui::InputFloat("Shininess", &selectedLight->shininess, 1.0f);

  float oldAngle = selectedLight->halfFieldOfView;
  ImGui::InputFloat("Half angle (outer-half)(spot Light)", &selectedLight->halfFieldOfView, 0.5f);

  if (oldAngle != selectedLight->halfFieldOfView)
  {
    selectedLight->halfFieldOfView = selectedLight->halfFieldOfView < 0.0f ? 0.0f : selectedLight->halfFieldOfView;
    selectedLight->halfFieldOfView = selectedLight->halfFieldOfView > 45.0f ? 45.0f : selectedLight->halfFieldOfView;
  }

  float oldInner = selectedLight->innerAngle;
  ImGui::InputFloat("Inner angle (spot Light)", &selectedLight->innerAngle, 0.5f);

  if (oldInner != selectedLight->innerAngle)
  {
    selectedLight->innerAngle = selectedLight->innerAngle < 0.0f ? 0.0f : selectedLight->innerAngle;
    selectedLight->innerAngle = selectedLight->innerAngle >= selectedLight->halfFieldOfView ? selectedLight->halfFieldOfView - 0.1f : selectedLight->innerAngle;
  }

  float oldFallOff = selectedLight->fallOff;
  ImGui::InputFloat("Fall Off (Spot Light)", &selectedLight->fallOff, 0.1f);

  if (oldFallOff != selectedLight->fallOff)
  {
    selectedLight->fallOff = selectedLight->fallOff < 0.0f ? 0.0f : selectedLight->fallOff;
  }


  float position[3] = { selectedLight->position.x, selectedLight->position.y, selectedLight->position.z };
  ImGui::InputFloat3("position (spot/point)", position);
  selectedLight->position.x = position[0];
  selectedLight->position.y = position[1];
  selectedLight->position.z = position[2];

  float direction[3] = { selectedLight->direction.x, selectedLight->direction.y, selectedLight->direction.z };
  ImGui::InputFloat3("direction (spot/directional)", direction);
  selectedLight->direction.x = direction[0];
  selectedLight->direction.y = direction[1];
  selectedLight->direction.z = direction[2];

  ImGui::Text("Point Light - 0");
  ImGui::Text("Directional Light - 1");
  ImGui::Text("Spot Light - 2");
  ImGui::InputInt("Light Type", &selectedLight->LightType);

  if (selectedLight->LightType > 2)
  {
    selectedLight->LightType = 2;
  }

  if (selectedLight->LightType < 0)
  {
    selectedLight->LightType = 0;
  }
}

void Editor::TextureMenu(const ImVec2& screenSize)
{
  ImGui::Checkbox("Toggle Textured", &centralObject->texture);

  if (ImGui::Button("cylindrical mapping"))
  {
    textureType = CYLIN;
    currentMapping = CYLIN;

    UpdateUVBuffer();
  }
  ImGui::SameLine();

  if (ImGui::Button("spherical mapping"))
  {
    textureType = SPHE;
    currentMapping = SPHE;

    UpdateUVBuffer();
  }
  ImGui::SameLine();

  if (ImGui::Button("Planar mapping"))
  {
    textureType = PLAN;
    currentMapping = PLAN;

    UpdateUVBuffer();
  }

  if (ImGui::Button("Texture Entity : Position"))
  {
    centralObject->textureEntity = POS;
    currentEntity = POS;

    UpdateUVBuffer();
  }

  if (ImGui::Button("Texture Entity : Normal"))
  {
    centralObject->textureEntity = NORM;
    currentEntity = NORM;

    UpdateUVBuffer();
  }

  ImGui::Checkbox("Use CPU Generated UV", &CPUUV);
}

void UpdateUVBuffer()
{
  std::vector<glm::vec2>* uv = &UVPlanarPos;

  if (currentMapping == CYLIN)
  {
    if (currentEntity == POS)
    {
      uv = &UVCylindricalPos;
    }
    else if (currentEntity == NORM)
    {
      uv = &UVCylindricalNorm;
    }
  }
  else if (currentMapping == SPHE)
  {
    if (currentEntity == POS)
    {
      uv = &UVSphericalPos;
    }
    else if (currentEntity == NORM)
    {
      uv = &UVSphericalNorm;
    }
  }
  else if (currentMapping == PLAN)
  {
    if (currentEntity == POS)
    {
      uv = &UVPlanarPos;
    }
    else if (currentEntity == NORM)
    {
      uv = &UVPlanarNorm;
    }
  }

  glNamedBufferSubData(centralObject->mesh->TBO, 0, sizeof(glm::vec2) * uv->size(), uv->data());
}