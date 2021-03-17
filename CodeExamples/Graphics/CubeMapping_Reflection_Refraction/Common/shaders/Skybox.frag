/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Skybox.frag, shader for skybox
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 15-11-19
*/

#version 460 core

out vec3 color;

in vec3 texEntity;

uniform sampler2D texTop;
uniform sampler2D texDown;
uniform sampler2D texRight;
uniform sampler2D texLeft;
uniform sampler2D texFront;
uniform sampler2D texBack;

#define TOP 5
#define DOWN 6
#define RIGHT 7
#define LEFT 8
#define FRONT 9
#define BACK 10

int GetPlane(vec3 entity)
{
  vec3 abs = abs(entity);
  int temp = 0;

  if (abs.x >= abs.y && abs.x >= abs.z)
  {
    if(entity.x > 0)
    {
      temp = LEFT;
    }
    else
    {
      temp = RIGHT;
    }
  }
  else if (abs.y >= abs.x && abs.y >= abs.z)
  {
    if(entity.y > 0)
    {
      temp = TOP;
    }
    else
    {
      temp = DOWN;
    }
  }
  else if (abs.z >= abs.x && abs.z >= abs.y)
  {
    if(entity.z > 0)
    {
      temp = FRONT;
    }
    else
    {
      temp = BACK;
    }
  }

  return temp;
}

vec2 CalculateCubeUV(vec3 entity)
{
  vec2 temp = vec2(0.0f);
  vec3 abs = abs(entity);

  if (abs.x >= abs.y && abs.x >= abs.z)
  {
    (entity.x < 0.0 ) ? ( temp.x = entity.z) : ( temp.x = -entity.z);
    temp.y = entity.y;
    temp = temp / abs.x;
  }
  else if (abs.y >= abs.x && abs.y >= abs.z)
  {
    (entity.y < 0.0 ) ? ( temp.y = entity.z) : ( temp.y = -entity.z);
    temp.x = entity.x;
    temp = temp / abs.y;
  }
  else if (abs.z >= abs.x && abs.z >= abs.y)
  {
    (entity.z < 0.0 ) ? ( temp.x = -entity.x) : ( temp.x = entity.x);
    temp.y = entity.y;
    temp = temp / abs.z;
  }

  temp += 1.0f;
  temp *= 0.5f;

  return temp;
}

vec3 GetColourFromTexture(vec2 uv, int plane)
{
  if(plane == TOP)
  {
    return texture(texTop, uv).xyz;
  }
  if(plane == DOWN)
  {
    return texture(texDown, uv).xyz;
  }
  if(plane == RIGHT)
  {
    return texture(texRight, uv).xyz;
  }
  if(plane == LEFT)
  {
    return texture(texLeft, uv).xyz;
  }
  if(plane == FRONT)
  {
    return texture(texFront, uv).xyz;
  }
  if(plane == BACK)
  {
    return texture(texBack, uv).xyz;
  }
}

void main()
{
  int plane = GetPlane(texEntity);
  vec2 UV = CalculateCubeUV(texEntity);
  color = GetColourFromTexture(UV, plane);
}
