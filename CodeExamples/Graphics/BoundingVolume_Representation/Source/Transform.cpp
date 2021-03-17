/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Transform.cpp, implementation for transform class (updates model matrix)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 3-10-19
*/

#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>

Transform::Transform() : position(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f),
rotation(0.0f, 0.0f, 0.0f), transformMatrix(1.0f), RotationMatrix(1.0f)
{}

void Transform::Update()
{
  RotationMatrix = glm::mat4(1.0f);
  RotationMatrix = glm::rotate(RotationMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
  RotationMatrix = glm::rotate(RotationMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
  RotationMatrix = glm::rotate(RotationMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

  transformMatrix = glm::mat4(1.0f);
  transformMatrix = glm::translate(transformMatrix, position);
  transformMatrix = transformMatrix * RotationMatrix;
  transformMatrix = glm::scale(transformMatrix, scale);
}