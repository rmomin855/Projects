#include "Sphere.h"
#include "Object.h"
#include "PhysicsManager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

Sphere::Sphere(Transform* transform, float radius)
  : Collider(transform, ColliderType::Sphere), sp_center(), sp_radius(radius)
{}

void Sphere::Update(float dt)
{
  if (!c_parent)
  {
    return;
  }

  Transform* tranform = c_parent->GetTransform();
  sp_center = tranform->GetPosition();

  if (glm::length2(tranform->GetScale()) != powf(sp_radius, 2.0f))
  {
    sp_radius = glm::length(tranform->GetScale());
  }

  c_node->UpdateNode(sp_center - sp_radius, sp_center + sp_radius);

  Collider::Update();
}
