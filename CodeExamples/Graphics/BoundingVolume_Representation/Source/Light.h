/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Light.cpp, decleration for light structure
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 15-11-19
*/

#pragma once
#include <glm/glm.hpp>

using namespace glm;

#define POINTLIGHT 0
#define DIRECTIONALLIGHT 1
#define SPOTLIGHT 2

typedef struct Light
{
  Light(const vec3& Position, const vec3& Direction) : position(Position), direction(Direction), halfFieldOfView(13.0f), diffuseColour(), LightType(POINTLIGHT)
    ,ambientStrength(0.01f), diffuseStrength(0.5f), specularStrength(0.8f), shininess(64.0f), innerAngle(0.0f), fallOff(1.0f), padding(), ambientColour(), specularColour()
  {}

  vec3 position;
  int LightType;

  vec3 direction;
  float halfFieldOfView;

  vec3 ambientColour;
  float ambientStrength;

  vec3 diffuseColour;
  float diffuseStrength;

  vec3 specularColour;
  float specularStrength;

  float padding;
  float shininess;
  float innerAngle;
  float fallOff;
}Light;