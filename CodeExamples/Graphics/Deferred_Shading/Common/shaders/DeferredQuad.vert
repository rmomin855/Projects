/*
Copyright (C) 2020 DigiPen Institute of Technology.
set up quad info to write from g-buffer
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS350
Author: Rahil Momin, r.momin, 180001717
Creation date: 31-1-20
*/

#version 460 core

in layout(location = 0) vec3 modelPosition;

uniform mat4 vertexTransform;

out vec2 quadCoord;

void main()
{
  gl_Position = vertexTransform * vec4(modelPosition, 1.0f);

  quadCoord = modelPosition.xy;
}