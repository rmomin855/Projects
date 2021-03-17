/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: PhongLighting.frag, passes colour forward
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 15-11-19
*/

#version 460 core

in vec3 colour;
out vec3 fragColor;

void main()
{
  fragColor = colour;
}
