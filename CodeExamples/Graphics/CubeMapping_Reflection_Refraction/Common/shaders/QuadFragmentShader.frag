/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: QuadVertexShader.frag, simple colour shader
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 15-11-19
*/

#version 460 core

// Input from the rasterizer
// Conceptually equivalent to
// "WritePixel( x, y, rasterColor )"

// Output data
out vec3 color;

void main()
{
	color = vec3(0.564f, 0.0f, 0.125f);
}
