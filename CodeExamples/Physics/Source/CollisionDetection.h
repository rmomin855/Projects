#pragma once
#include "Generic.h"
#include "Sphere.h"
#include "CollisionHelperFunctions.h"
#include "Transform.h"

using namespace glm;
#define MAX_ITERATIONS 50

void EPA(std::vector<vec3>& simplex, std::vector<vec3>& points1, std::vector<vec3>& points2, ContactManifold& manifold);

static bool CheckAABB(Collider* collider1, Collider* collider2)
{
  Generic* aabb1 = reinterpret_cast<Generic*>(collider1);
  Generic* aabb2 = reinterpret_cast<Generic*>(collider2);

  vec3 max1 = aabb1->ab_center + aabb1->ab_halfExtend;
  vec3 min2 = aabb2->ab_center - aabb2->ab_halfExtend;

  //if any min is greater than others max in any axis then they cannot be intersecting
  if (min2.x > max1.x || min2.y > max1.y || min2.z > max1.z)
  {
    return false;
  }

  vec3 min1 = aabb1->ab_center - aabb1->ab_halfExtend;
  vec3 max2 = aabb2->ab_center + aabb2->ab_halfExtend;

  if (min1.x > max2.x || min1.y > max2.y || min1.z > max2.z)
  {
    return false;
  }

  return true;
}

static bool CheckSphere(Collider* collider1, Collider* collider2)
{
  Sphere* sphere1 = reinterpret_cast<Sphere*>(collider1);
  Sphere* sphere2 = reinterpret_cast<Sphere*>(collider2);

  vec3 distVec = sphere1->sp_center - sphere2->sp_center;
  float radius = sphere1->sp_radius + sphere2->sp_radius;

  return glm::length2(distVec) < powf(radius, 2.0);
}

static bool GJK(Collider* collider1, Collider* collider2)
{
  std::vector<vec3> points1 = collider1->GetParent()->GetModel()->GetMesh()->vertices;
  std::vector<vec3> points2 = collider2->GetParent()->GetModel()->GetMesh()->vertices;
  const mat4& matrix1 = collider1->GetParent()->GetTransform()->GetModelMatrix();
  const mat4& matrix2 = collider2->GetParent()->GetTransform()->GetModelMatrix();

  std::vector<vec3> simplex;
  bool collided = false;

  TransformPoints(points1, matrix1);
  TransformPoints(points2, matrix2);

  vec3 dir = points1[0] - points2[0];
  if (!length2(dir))
  {
    dir.y = 1.0f;
  }

  vec3 supportDiff = GetSupportDifference(points1, points2, dir);
  vec3 supportDiffTemp = GetSupportDifference(points1, points2, -dir);

  dir = (length2(supportDiff) < length2(supportDiffTemp)) ? -supportDiff : -supportDiffTemp;
  supportDiff = GetSupportDifference(points1, points2, dir);

  //if the difference in support points is further than the point in negative dir, then not colliding
  //dir dot point == 0.0f;
  if (dot(supportDiff, dir) < 0.0f)
  {
    return false;
  }

  dir = -supportDiff;
  simplex.push_back(supportDiff);

  size_t iterations = 0;
  while (true)
  {
    ++iterations;
    if (iterations > MAX_ITERATIONS)
    {
      break;
    }

    //get closest point in simplex to the origin
    //reduce simplex to a subset that has the closest point
    vec3 pointClosestToOrigin = GetClosestPointToOrigin(simplex);

    //if its the origin itself then colliding
    if (pointClosestToOrigin == vec3(0.0f))
    {
      collider1->GetParent()->GetModel()->SetColour(0.0f, 1.0f, 0.0f);
      collided = true;

      dir = -pointClosestToOrigin;
      size_t size = simplex.size();
      for (size_t i = size; i < 4; i++)
      {
        supportDiff = GetSupportDifference(points1, points2, dir);
        simplex.push_back(supportDiff);
        dir = cross(dir, -supportDiff);
      }

      break;
    }

    dir = -pointClosestToOrigin;
    supportDiff = GetSupportDifference(points1, points2, dir);

    //if the difference in support points is further than the point in negative dir, then not colliding
    if (dot(supportDiff, dir) < 0.0f)
    {
      break;
    }

    simplex.push_back(supportDiff);
  }

  if (collided)
  {
    //EPA
    ContactManifold manifold;
    EPA(simplex, points1, points2, manifold);
    PhysicsManager::Instance()->AddContactManifold(collider1, collider2, manifold);
  }

  return collided;
}

void EPA(std::vector<vec3>& simplex, std::vector<vec3>& points1, std::vector<vec3>& points2, ContactManifold& manifold)
{
  vec3 triNormal = vec3();
  float distance = 0.0f;

  std::vector<Face> faces;
  faces.push_back({ simplex[0], simplex[1], simplex[2] });
  faces.push_back({ simplex[0], simplex[1], simplex[3] });
  faces.push_back({ simplex[0], simplex[2], simplex[3] });
  faces.push_back({ simplex[1], simplex[2], simplex[3] });

  size_t iterations = 0;
  while (true)
  {
    ++iterations;
    if (iterations > MAX_ITERATIONS)
    {
      break;
    }

    FindClosestFace(faces, triNormal, distance);

    vec3 support1 = GetSupportPoint(points1, -triNormal);
    vec3 support2 = GetSupportPoint(points2, triNormal);
    vec3 supportDifferece = support1 - support2;

    manifold.depth = dot(supportDifferece, triNormal);
    if (manifold.depth - distance < TOLERANCE)
    {
      manifold.cp_World1 = support1;
      manifold.cp_World2 = support2;
      manifold.cp_penVec = triNormal;
      CalculateTangents(triNormal, manifold);
      return;
    }

    ReconstructFaces(faces, supportDifferece);
  }

  manifold.cp_penVec = vec3(0.0f);
}