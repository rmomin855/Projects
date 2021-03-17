#pragma once
#include "Collider.h"
class Sphere : public Collider
{
public:
  Sphere(Transform* transform, float radius);

  void Update(float dt = 0.0f);

  //members
  float sp_radius;
  glm::vec3 sp_center;
};

