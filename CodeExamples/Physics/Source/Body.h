#ifndef BODY
#define BODY
#pragma once

#include <glm/glm.hpp>
#include "Transform.h"

using namespace glm;

class Object;
class Body
{
public:
  Body(Transform* transform, Model* model, float mass = 1.0f);
  ~Body();

  void SetParent(Object* parent);
  Object* GetParent();

  void Update(float dt);
  void RecalculateInertiaTensor(Model* model);

  void DrawDebug();

  //members
  bool b_gravity;
  float b_mass;
  vec3 b_force;
  vec3 b_velocity;

  //angular
  vec3 b_torque;
  vec3 b_angularVelocity;

  vec3 b_oldPosition;
  vec3 b_oldRotation;
  mat3 b_inertia;
  mat3 b_inertiaInv;

private:
  Object* b_Parent;
  Transform* b_transform;
};


#endif // !BODY
