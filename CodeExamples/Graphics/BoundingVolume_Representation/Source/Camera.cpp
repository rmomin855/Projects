/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Camera.cpp, implementation for camera class (first person view)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS350
Author: Rahil Momin, r.momin, 180001717
Creation date: 7-3-20
*/
#include "Camera.h"

Camera::Camera(const vec3& positon)
  : cam_position(positon), cam_front(0.0f, 0.0f, -1.0f), cam_right(), cam_up(0.0f, 1.0f, 0.0f), cam_viewMatrix(),
  cam_dirty(false), cam_speed(600.0f), pitch(0.0f), yaw(-90.0f), cam_mouseSpeed(0.5f)
{
  vec3 direction = normalize(-cam_position);
  cam_right = cross(cam_up, direction);

  cam_viewMatrix = lookAt(cam_position, cam_position + cam_front, cam_up);
}

void Camera::Update()
{
  if (cam_dirty)
  {
    cam_viewMatrix = lookAt(cam_position, cam_position + cam_front, cam_up);
  }
}

const mat4& Camera::GetViewMatrix()
{
  return cam_viewMatrix;
}

void Camera::SetPosition(const vec3& position)
{
  cam_position = position;
  cam_dirty = true;
}

void Camera::Input(GLFWwindow* window)
{
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  {
    cam_position += cam_speed * cam_front;
    cam_dirty = true;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
  {
    cam_position -= cam_speed * cam_front;
    cam_dirty = true;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
  {
    cam_position += cam_speed * cam_right;
    cam_dirty = true;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
  {
    cam_position -= cam_speed * cam_right;
    cam_dirty = true;
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
  {
    cam_position += cam_speed * cam_up;
    cam_dirty = true;
  }
  if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
  {
    cam_position -= cam_speed * cam_up;
    cam_dirty = true;
  }

  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
  {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    MouseInput(vec2(x, y), false);
  }
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
  {
    MouseInput(vec2(), true);
  }
}

void Camera::MouseInput(vec2 cursorPos, bool end)
{
  static float lastX = 0;
  static float lastY = 0;

  if (end)
  {
    lastX = 0;
    lastY = 0;
    return;
  }
  if (!lastX || !lastY)
  {
    lastX = cursorPos.x;
    lastY = cursorPos.y;
    return;
  }

  float xOffSet = cursorPos.x - lastX;
  float yOffSet = lastY - cursorPos.y;
  xOffSet *= cam_mouseSpeed;
  yOffSet *= cam_mouseSpeed;

  yaw += xOffSet;
  pitch += yOffSet;

  if (pitch > 89.0f)
  {
    pitch = 89.0f;
  }
  if (pitch < -89.0f)
  {
    pitch = -89.0f;
  }

  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(pitch));
  direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cam_front = glm::normalize(direction);

  cam_right = cross(cam_up, cam_front);

  lastX = cursorPos.x;
  lastY = cursorPos.y;
  cam_dirty = true;
}
