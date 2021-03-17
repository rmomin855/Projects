/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Skybox.h, decleration for skybox
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 3-10-19
*/

#ifndef SKYBOX
#define SKYBOX
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Object.h"

#include <vector>
#include <string>

#define TOP 0
#define DOWN 1
#define RIGHT 2
#define LEFT 3
#define FRONT 4
#define BACK 5

#define TOPTEX 5
#define DOWNTEX 6
#define RIGHTTEX 7
#define LEFTTEX 8
#define FRONTTEX 9
#define BACKTEX 10

class Skybox : public Object
{
public:

  std::vector<GLuint> textureId;

  Skybox(std::vector<std::string>& textures);

  GLuint CreateTexture(std::string& texture);

  void Draw();

  void uniforms(GLuint programID);
};

#endif // !SKYBOX
