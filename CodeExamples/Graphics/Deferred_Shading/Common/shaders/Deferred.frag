/*
Copyright (C) 2020 DigiPen Institute of Technology.
calculate uv and write to the g-buffer
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS350
Author: Rahil Momin, r.momin, 180001717
Creation date: 31-1-20
*/

#version 460 core

layout (location = 0) out vec3 Position;
layout (location = 1) out vec3 Normal;
layout (location = 2) out vec3 UV;

in vec3 fragNormal;
in vec3 position;
in vec2 uv;
in vec3 posUV;

uniform vec3 maxPoint;
uniform vec3 minPoint;
uniform int textureType;
uniform int CPUGenUV;

uniform vec3 modelColour;

vec2 GetUV();

out vec3 colour;

void main()
{
  Position = position;
  Normal = fragNormal;
  UV = vec3(GetUV(), 0.0f);

  colour = modelColour;
}

vec2 CalculateCubeUV(vec3 entity)
{
  vec2 temp = vec2(0.0f);
  vec3 abs = abs(entity);


  if (abs.x >= abs.y && abs.x >= abs.z)
  {
    temp.x = entity.y / abs.x;
    temp.y = entity.z / abs.x;
  }
  else if (abs.y >= abs.x && abs.y >= abs.z)
  {
    temp.x = entity.x / abs.y;
    temp.y = entity.z / abs.y;
  }
  else if (abs.z >= abs.x && abs.z >= abs.y)
  {
    temp.x = entity.x / abs.z;
    temp.y = entity.y / abs.z;
  }

  temp += 1.0f;
  temp *= 0.5f;

  return temp;
}

vec2 GetUV()
{
  if(CPUGenUV == 1)
  {  
    return uv;
  }

  vec3 temp = posUV;
  vec3 max = maxPoint;
  vec3 min = minPoint;

  float angle = atan(temp.y, temp.x);
  angle *= 2.0f;
  angle = angle + radians(180.0f);

  float z = (temp.z - min.z) / (max.z - min.z);

  float u = angle / radians(360.0f);
  float v = 0;

  //cylindrical
  if(textureType == 0)
  {
   v = z;
  }

  //spherical
  else if(textureType == 1)
  {
    float phi = acos(z / length(temp));
    phi /= radians(180.0f);

    v = 1.0f - phi;
  }

  //planar
  else if(textureType == 2)
  {
    vec2 uv = CalculateCubeUV(temp);
    u = uv.x;
    v = uv.y;
  }

  return vec2(u, v);
}