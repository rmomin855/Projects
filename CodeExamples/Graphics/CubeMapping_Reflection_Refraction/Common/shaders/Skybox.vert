/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Skybox.vert, shader for skybox
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 15-11-19
*/

#version 460 core

uniform mat4 vertexTransform;
uniform mat4 ProjTransform;

// Input vertex data, different for all executions of this shader.
in layout(location = 0) vec3 modelPosition;
in layout(location = 1) vec3 vertexNormal;
in layout(location = 2) vec2 UV;

out vec3 texEntity;

void main()
{
    vec4 vPos = vec4( modelPosition.x, modelPosition.y, modelPosition.z, 1.0f );
    gl_Position = (ProjTransform * vertexTransform) * vPos;
    texEntity = modelPosition;
}
