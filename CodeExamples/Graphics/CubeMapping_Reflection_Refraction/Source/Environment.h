/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Envionment.h, decleration for FBO
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 3-10-19
*/

#ifndef ENVIRONMENT
#define ENVIRONMENT

#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

enum class environmentEntity
{
  reflection = 0,
  refraction,
  mix
};

class Environment
{
public:
  GLuint FBO;
  GLuint RBO;

  GLuint Top;
  GLuint Bottom;
  GLuint Left;
  GLuint Right;
  GLuint Front;
  GLuint Back;

  Environment();
  void Update();
  void SendToShader(GLuint programID);

  environmentEntity mapping;
  float refectionIndex;
  float fresnal;
};
#endif // !ENVIRONMENT
