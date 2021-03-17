/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Camera.h, decleration for camera class (first person view)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS350
Author: Rahil Momin, r.momin, 180001717
Creation date: 7-3-20
*/
#ifndef CAMERA
#define CAMERA

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

using namespace glm;

class InputManager;
class Camera
{
public:
  Camera(const vec3& positon = vec3(0.0f, 0.0f, 5.0f));
  void Update();

  const mat4& GetViewMatrix();

  void SetPosition(const vec3& position);

  void Input(GLFWwindow* window);
  void MouseInput(vec2 cursorPos, bool end);
  friend class InputManager;

  vec3 cam_position;
  vec3 cam_front;
  vec3 cam_right;
  vec3 cam_up;

  float pitch;
  float yaw;

  mat4 cam_viewMatrix;
  bool cam_dirty;
  float cam_speed;
  float cam_mouseSpeed;
};

#endif // !CAMERA
