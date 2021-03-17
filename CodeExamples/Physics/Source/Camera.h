#ifndef CAMERA
#define CAMERA

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class InputManager;
class Camera
{
public:
  Camera(const vec3& positon = vec3(0.0f, 0.0f, 5.0f));
  void Update();

  const mat4& GetViewMatrix();

  void SetPosition(const vec3& position);

  void MouseInput(vec2 cursorPos, bool end);
  friend class InputManager;
private:
  vec3 cam_position;
  vec3 cam_front;
  vec3 cam_right;
  vec3 cam_up;

  float pitch;
  float yaw;

  mat4 cam_viewMatrix;
  bool cam_dirty;
  float cam_speed;
};

#endif // !CAMERA
