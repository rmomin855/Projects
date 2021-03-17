/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Object.h, declaration for Object class (holds object information)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 3-10-19
*/

#pragma once
#include <string>
#include "Transform.h"
#include "MeshManager.h"

enum class Shaders
{
  phongShading,
  phongLighting,
  blinnShading,
  simple,
  skybox
};

#define DIFFUSE 5
#define SPECULAR 6

class Object
{
public:
  Object(const char* Name);
  ~Object();

  std::string name;
  Transform transform;

  Mesh* mesh;
  DrawType drawType;
  glm::vec3 colour;
  Shaders shaderToUse;

  GLuint UBO;
  bool texture;
  int textureEntity;

  bool rotateObject;

  void Update();
  virtual void Draw(GLuint programID);
  void DrawFaceNormal(GLuint programid);
  void DrawVertexNormal(GLuint programid);

  void SetUniforms(GLuint programID);
};