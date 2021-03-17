/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Transform.h, declaration for transform class (holds object world information)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 3-10-19
*/

#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class Transform
{
public:
  Transform();

  void Update();

  glm::vec3 position;
  glm::vec3 scale;
  glm::vec3 rotation;

  glm::mat4 transformMatrix;
  glm::mat4 RotationMatrix;
};