#pragma once
#include "Collider.h"

class Generic : public Collider
{
public:
  Generic(Transform* transform, const glm::vec3& scale);

  void Update(float dt = 0.0f);

  //members
  glm::vec3 ab_halfExtend;
  glm::vec3 ab_center;
};

