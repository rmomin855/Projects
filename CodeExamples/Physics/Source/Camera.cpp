#include "Camera.h"

Camera::Camera(const vec3& positon)
  : cam_position(positon), cam_front(0.0f, 0.0f, -1.0f), cam_right(), cam_up(0.0f, 1.0f, 0.0f), cam_viewMatrix(),
  cam_dirty(false), cam_speed(0.5f), pitch(0.0f), yaw(-90.0f)
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
  xOffSet *= cam_speed;
  yOffSet *= cam_speed;

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
