#pragma once
#ifndef TRANSFORM
#define TRANSFORM

#include <glm/glm.hpp>

using namespace glm;

class Object;
class Transform
{
public:
  Transform(const vec3& position = vec3(), const vec3& scale = vec3(1.0f, 1.0f, 1.0f), const vec3& rotation = vec3(0.0f, 0.0f, 0.0f));
  ~Transform();

  Object* GetParent();
  const vec3& GetPosition();
  const vec3& GetScale();
  const vec3& GetRotations();
  const mat4& GetModelMatrix();
  const mat4& GetRotationMatrix();

  bool GetHasChangedStatus();
  bool GetScaleChangedStatus();

  void UpdatePosition(const vec3& position);
  void UpdatePosition(float x, float y, float z);
  void UpdateScale(const vec3& scale);
  void UpdateScale(float x, float y, float z);
  void UpdateRotation(const vec3& rotation);
  void UpdateRotation(float x, float y, float z);
  void RotateCurrentbyX(float x);
  void RotateCurrentbyY(float y);
  void RotateCurrentbyZ(float z);

  void SetParent(Object* parent);
  void SetPosition(const vec3& position);
  void SetPosition(float x, float y, float z);
  void SetScale(const vec3& scale);
  void SetScale(float x, float y, float z);
  void SetRotation(const vec3& rotation);
  void SetRotation(float x, float y, float z);

  void Update(float dt = 0.0f);

private:
  Object* m_parent;
  vec3 m_position;
  vec3 m_scale;
  vec3 m_rotation;

  mat4 m_modelMatrix;
  mat4 m_rotationMatrix;

  bool m_isDirty;
  bool m_hasChanged;
  bool m_scaleChanged;
};

#endif // !TRANSFORM
