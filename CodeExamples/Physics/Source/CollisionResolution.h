#pragma once
#include "PhysicsManager.h"

void ApplyPenVec(vec3& penVec, Transform* transform1, Transform* transform2)
{
  if (length2(penVec))
  {
    vec3 posVec = transform1->GetPosition() - transform2->GetPosition();

    Body* body1 = transform1->GetParent()->GetBody();
    Body* body2 = transform2->GetParent()->GetBody();

    if (body1->b_mass && body2->b_mass)
    {
      penVec /= 2.0f;
    }

    if (dot(posVec, penVec) < 0.0f)
    {
      penVec = -penVec;
    }

    body1->b_mass ? transform1->UpdatePosition(penVec) : transform1->UpdatePosition(vec3());
    body2->b_mass ? transform2->UpdatePosition(-penVec) : transform2->UpdatePosition(vec3());
  }
}

static void ResolveCollision(Collider* collider1, Collider* collider2)
{
  ContactManifold& manifold = PhysicsManager::Instance()->GetContactManifold(collider1, collider2);
  vec3 penVec = manifold.cp_penVec * manifold.depth;

  Transform* transform1 = collider1->GetParent()->GetTransform();
  Transform* transform2 = collider2->GetParent()->GetTransform();

  Body* body1 = transform1->GetParent()->GetBody();
  Body* body2 = transform2->GetParent()->GetBody();

  vec3 pos1 = transform1->GetPosition();
  vec3 pos2 = transform2->GetPosition();

  ApplyPenVec(penVec, transform1, transform2);

  vec3 posVec = transform1->GetPosition() - transform2->GetPosition();
  if (dot(posVec, manifold.cp_penVec) < 0.0f)
  {
    manifold.cp_penVec = -manifold.cp_penVec;
  }

  float relVelocity = dot(manifold.cp_penVec, body1->b_velocity - body2->b_velocity);

  if (relVelocity >= 0.0f)
  {
    return;
  }

  GraphicsManager::Instance()->DrawDebugLine(manifold.cp_World1, manifold.cp_World1 + penVec, Colours::Purple);
  GraphicsManager::Instance()->DrawDebugPoint(manifold.cp_World1, Colours::Purple);

  vec3 posToPoi1 = manifold.cp_World1 - transform1->GetPosition(); //ra
  vec3 posToPoi2 = manifold.cp_World2 - transform2->GetPosition(); //rb

  float num = -1.9f * relVelocity;

  float denum1 = body1->b_mass ? 1.0f / body1->b_mass : 0.0f;
  float denum2 = body2->b_mass ? 1.0f / body2->b_mass : 0.0f;
  float denum3 = body1->b_mass ? dot(manifold.cp_penVec, cross(body1->b_inertiaInv * cross(posToPoi1, manifold.cp_penVec), posToPoi1)) : 0.0f;
  float denum4 = body2->b_mass ? dot(manifold.cp_penVec, cross(body2->b_inertiaInv * cross(posToPoi2, manifold.cp_penVec), posToPoi2)) : 0.0f;

  float impulse = num / (denum1 + denum2 + denum3 + denum4);

  if (body1->b_mass)
  {
    vec3 velocity = impulse * manifold.cp_penVec / body1->b_mass;
    body1->b_velocity += velocity;
    vec3 angular = body1->b_inertiaInv * cross(posToPoi1, impulse * manifold.cp_penVec);
    body1->b_angularVelocity += angular;

    GraphicsManager::Instance()->DrawDebugLine(pos1, pos1 + velocity * 5.0f, Colours::Orange);
    GraphicsManager::Instance()->DrawDebugLine(pos1, pos1 + angular * 5.0f, Colours::Purple);
  }
  if (body2->b_mass)
  {
    vec3 velocity = -impulse * manifold.cp_penVec / body2->b_mass;
    body2->b_velocity += velocity;
    vec3 angular = body2->b_inertiaInv * cross(posToPoi2, -impulse * manifold.cp_penVec);
    body2->b_angularVelocity += angular;

    GraphicsManager::Instance()->DrawDebugLine(pos2, pos2 + velocity * 5.0f, Colours::Orange);
    GraphicsManager::Instance()->DrawDebugLine(pos2, pos2 + angular * 5.0f, Colours::Purple);
  }
}