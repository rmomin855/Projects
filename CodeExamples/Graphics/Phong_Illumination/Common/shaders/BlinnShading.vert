/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: BlinnShading.vert, implementation of phongShading, calculates
normals and approprite positions
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 15-11-19
*/

#version 460 core

uniform mat4 vertexTransform;
uniform mat4 RotTransform;
uniform mat4 ProjTransform;

uniform int textureEntity;

in layout(location = 0) vec3 modelPosition;
in layout(location = 1) vec3 vertexNormal;
in layout(location = 2) vec2 UV;

out vec3 fragNormal;
out vec3 position;
out vec2 uv;
out vec3 posUV;

void main()
{
  vec4 tempNorm = RotTransform * vec4(vertexNormal, 0.0f);
  
  fragNormal = tempNorm.xyz;

  gl_Position = (ProjTransform * vertexTransform) * vec4( modelPosition, 1.0f );
  position = (vertexTransform * vec4( modelPosition, 1.0f )).xyz;

  uv = UV;
  posUV = modelPosition;

  //entity = normal
  if(textureEntity == 1)
  {
    posUV = normalize(vertexNormal);
  }
}
