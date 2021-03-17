#include "Object.h"
#include "GraphicsManager.h"
#include "PhysicsManager.h"
#include "Jacobi.h"

#include <Windows.h>
#include <ppl.h>

Body::Body(Transform* transform, Model* model, float mass)
  : b_Parent(), b_transform(transform), b_mass(mass), b_force(), b_velocity(), b_angularVelocity(), b_oldPosition(), b_oldRotation(),
  b_gravity(true), b_inertia(), b_torque(), b_inertiaInv()
{
  PhysicsManager::Instance()->AddToList(this);
  RecalculateInertiaTensor(model);
}

Body::~Body()
{
  PhysicsManager::Instance()->RemoveFromList(this);
}

void Body::SetParent(Object* parent)
{
  b_Parent = parent;
}

Object* Body::GetParent()
{
  return b_Parent;
}

void Body::Update(float dt)
{
  if (b_mass && b_Parent)
  {
    Transform* transform = b_Parent->GetTransform();

    b_oldPosition = transform->GetPosition();
    b_oldRotation = transform->GetRotations();

    if (b_gravity)
    {
      b_force += b_mass * PhysicsManager::pm_gravityForce;
    }

    b_velocity += (b_force / b_mass) * dt;

    //simple drag stuff
    //TAKE THIS OUT
    b_velocity -= PhysicsManager::pm_dragConstant * b_velocity;

    //angular
    b_angularVelocity += b_inertiaInv * b_torque * dt;
    b_angularVelocity -= PhysicsManager::pm_angularDragConstant * b_angularVelocity;

    transform->UpdatePosition(b_velocity * dt);
    transform->UpdateRotation(b_angularVelocity * dt);

    if (GraphicsManager::Instance()->GetDebugStatus())
    {
      DrawDebug();
    }

    b_force = vec3();
    b_torque = vec3();
  }
}

//works mostly best when object is more or less axis aligned
void Body::RecalculateInertiaTensor(Model* model)
{
  const vec3& scale = b_transform->GetScale();
  float x = scale.x;
  float y = scale.y;
  float z = scale.z;

  mat3 inertiaMatrix;
  float diagMass = b_mass / (x * y * z) / 3.0f;
  float otherMass = b_mass / (x * y * z) / 4.0f;

  inertiaMatrix[0][0] = diagMass * ((x * powf(y, 3.0f) * z) + (x * y * powf(z, 3.0f)));
  inertiaMatrix[1][1] = diagMass * ((y * powf(x, 3.0f) * z) + (x * y * powf(z, 3.0f)));
  inertiaMatrix[2][2] = diagMass * ((y * powf(x, 3.0f) * z) + (x * z * powf(y, 3.0f)));

  inertiaMatrix[0][1] = -otherMass * (powf(x, 2.0f) * powf(y, 2.0f) * z);
  inertiaMatrix[1][0] = -otherMass * (powf(x, 2.0f) * powf(y, 2.0f) * z);

  inertiaMatrix[0][2] = -otherMass * (powf(x, 2.0f) * powf(z, 2.0f) * y);
  inertiaMatrix[2][0] = -otherMass * (powf(x, 2.0f) * powf(z, 2.0f) * y);

  inertiaMatrix[1][2] = -otherMass * (powf(y, 2.0f) * powf(z, 2.0f) * x);
  inertiaMatrix[2][1] = -otherMass * (powf(y, 2.0f) * powf(z, 2.0f) * x);

  b_inertia = inertiaMatrix;
  b_inertiaInv = glm::inverse(inertiaMatrix);
}

void Body::DrawDebug()
{
  const vec3& position = b_transform->GetPosition();
  GraphicsManager::Instance()->DrawDebugLine(position, position + (b_velocity), Colours::Blue);
  GraphicsManager::Instance()->DrawDebugLine(position, position + (b_angularVelocity), Colours::Red);
}
