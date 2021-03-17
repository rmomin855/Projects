/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: QuadVertexShader.vert, simple colour shader
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 15-11-19
*/

#version 460 core

uniform mat4  vertexTransform;
uniform mat4  projTransform;

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3  vPosition;

void main()
{
    vec4 vPos = vec4( vPosition.x, vPosition.y, vPosition.z, 1.0f );
    gl_Position = (projTransform * vertexTransform) * vPos;
}
