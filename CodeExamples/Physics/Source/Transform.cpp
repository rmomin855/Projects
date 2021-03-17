#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include "PhysicsManager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

Transform::Transform(const vec3& position, const vec3& scale, const vec3& rotation)
  : m_position(position), m_scale(scale), m_rotation(rotation), m_modelMatrix(), m_rotationMatrix(),
  m_isDirty(true), m_parent(), m_hasChanged(false), m_scaleChanged(false)
{
  PhysicsManager::Instance()->AddToList(this);
}

Transform::~Transform()
{
  PhysicsManager::Instance()->RemoveFromList(this);
}

Object* Transform::GetParent()
{
  return m_parent;
}

const vec3& Transform::GetPosition()
{
  return m_position;
}

const vec3& Transform::GetScale()
{
  return m_scale;
}

const vec3& Transform::GetRotations()
{
  return m_rotation;
}

const mat4& Transform::GetModelMatrix()
{
  return m_modelMatrix;
}

const mat4& Transform::GetRotationMatrix()
{
  return m_rotationMatrix;
}

bool Transform::GetHasChangedStatus()
{
  return m_hasChanged;
}

bool Transform::GetScaleChangedStatus()
{
  return m_scaleChanged;
}

void Transform::UpdatePosition(const vec3& position)
{
  if (!glm::length2(position))
  {
    return;
  }

  m_position += position;
  m_isDirty = true;
}

void Transform::UpdatePosition(float x, float y, float z)
{
  m_position += vec3(x, y, z);
  m_isDirty = true;
}

void Transform::UpdateScale(const vec3& scale)
{
  m_scale += scale;
  m_isDirty = true;
  m_scaleChanged = true;
}

void Transform::UpdateScale(float x, float y, float z)
{
  m_scale += vec3(x, y, z);
  m_isDirty = true;
  m_scaleChanged = true;
}

void Transform::UpdateRotation(const vec3& rotation)
{
  m_rotation += rotation;
  m_isDirty = true;
}

void Transform::UpdateRotation(float x, float y, float z)
{
  m_rotation.x += x;
  m_rotation.y += y;
  m_rotation.z += z;

  m_isDirty = true;
}

void Transform::RotateCurrentbyX(float x)
{
  m_rotation.x += x;
  m_isDirty = true;
}

void Transform::RotateCurrentbyY(float y)
{
  m_rotation.y += y;
  m_isDirty = true;
}

void Transform::RotateCurrentbyZ(float z)
{
  m_rotation.z += z;
  m_isDirty = true;
}

void Transform::SetParent(Object* parent)
{
  m_parent = parent;
}

void Transform::SetPosition(const vec3& position)
{
  m_position = position;
  m_isDirty = true;
}

void Transform::SetPosition(float x, float y, float z)
{
  m_position = vec3(x, y, z);
  m_isDirty = true;
}

void Transform::SetScale(const vec3& scale)
{
  m_scale = scale;
  m_isDirty = true;
  m_scaleChanged = true;
}

void Transform::SetScale(float x, float y, float z)
{
  m_scale = vec3(x, y, z);
  m_isDirty = true;
  m_scaleChanged = true;
}

void Transform::SetRotation(const vec3& rotation)
{
  m_rotation = rotation;
  m_isDirty = true;
}

void Transform::SetRotation(float x, float y, float z)
{
  m_rotation = vec3(x, y, z);
  m_isDirty = true;

  m_rotation.x = m_rotation.x;
  m_rotation.y = m_rotation.y;
  m_rotation.z = m_rotation.z;
}

void Transform::Update(float dt)
{
  m_hasChanged = false;

  if (m_isDirty && m_parent)
  {
    m_rotationMatrix = glm::mat4(1.0f);
    m_rotationMatrix = glm::rotate(m_rotationMatrix, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    m_rotationMatrix = glm::rotate(m_rotationMatrix, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    m_rotationMatrix = glm::rotate(m_rotationMatrix, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    m_modelMatrix = glm::mat4(1.0f);
    m_modelMatrix = glm::translate(m_modelMatrix, m_position);
    m_modelMatrix = m_modelMatrix * m_rotationMatrix;
    m_modelMatrix = glm::scale(m_modelMatrix, m_scale);

    m_isDirty = false;
    m_hasChanged = true;
    m_scaleChanged = false;
  }
}