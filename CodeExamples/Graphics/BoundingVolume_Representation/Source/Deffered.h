/*
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Object.cpp, implementation for Object class (holds object information)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS350
Author: Rahil Momin, r.momin, 180001717
Creation date: 31-1-20
*/
#pragma once
#ifndef DEFFERED
#define DEFFERED

#include <vector>

#include "Object.h"

#define NUMTARGETS 3

class Model;
class Deferred
{
public:
  Deferred(size_t width, size_t height);
  void Render(const std::vector<Object*>& objects);
  void Render(std::vector<Mesh*>* meshes);

  void CopyDepthBuffer();

  GLuint deferedShader;
  bool copyDepth;

  int FSQTarget;

private:
  GLuint FBO;
  GLuint depth;

  GLuint width;
  GLuint height;

  GLuint renderTargets[NUMTARGETS] = {};
  GLuint attachments[NUMTARGETS] = {};
};

#endif // !DEFFERED