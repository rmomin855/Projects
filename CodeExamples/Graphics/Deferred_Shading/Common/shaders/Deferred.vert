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

in layout(location = 0) vec3 modelPosition;
in layout(location = 1) vec3 vertexNormal;
in layout(location = 2) vec2 UV;

uniform mat4 vertexTransform;
uniform mat4 RotTransform;
uniform mat4 ProjTransform;

uniform int textureEntity;

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