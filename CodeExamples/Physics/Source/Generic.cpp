#include "Generic.h"
#include "PhysicsManager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

using namespace glm;

Generic::Generic(Transform* transform, const glm::vec3& scale)
  : Collider(transform, ColliderType::Generic), ab_center(), ab_halfExtend(scale)
{}

void Generic::Update(float dt)
{
  if (!c_parent)
  {
    return;
  }

  Transform* tranform = c_parent->GetTransform();
  ab_center = tranform->GetPosition();
  ab_halfExtend = tranform->GetScale();

  if (length2(tranform->GetRotations()))
  {
    mat3 rotMat = mat3(tranform->GetRotationMatrix());
    rotMat[0] = glm::abs(rotMat[0]);
    rotMat[1] = glm::abs(rotMat[1]);
    rotMat[2] = glm::abs(rotMat[2]);

    ab_halfExtend = rotMat * ab_halfExtend;
  }

  c_node->UpdateNode(ab_center - ab_halfExtend, ab_center + ab_halfExtend);

  Collider::Update();
}