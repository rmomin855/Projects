#include "PhysicsManager.h"
#include "CollisionDetection.h"
#include "CollisionResolution.h"

#include <Windows.h>
#include <ppl.h>

PhysicsManager* PhysicsManager::pm_singleton = nullptr;

float PhysicsManager::pm_dragConstant = 0.01f;
float PhysicsManager::pm_angularDragConstant = 0.005f;
vec3 PhysicsManager::pm_gravityForce = vec3(0.0f, -9.81f, 0.0f);

void PhysicsManager::AddToList(Transform* transform)
{
  pm_transforms.push_back(transform);
}

void PhysicsManager::AddToList(Body* body)
{
  pm_bodies.push_back(body);
}

void PhysicsManager::AddToList(Collider* collider)
{
  pm_colliders.push_back(collider);
}

void PhysicsManager::AddToBVH(Node* node)
{
  if (!node)
  {
    return;
  }

  if (!bvh_root)
  {
    bvh_root = node;
    return;
  }

  bvh_root->AddToNode(node);
  numChanges += 1.0f;
  numNodes += 1.0f;
}

void PhysicsManager::RemoveFromList(Transform* transform)
{
  auto spot = std::find(pm_transforms.begin(), pm_transforms.end(), transform);
  pm_transforms.erase(spot);
}

void PhysicsManager::RemoveFromList(Body* body)
{
  auto spot = std::find(pm_bodies.begin(), pm_bodies.end(), body);
  pm_bodies.erase(spot);
}


void PhysicsManager::RemoveFromList(Collider* collider)
{
  auto spot = std::find(pm_colliders.begin(), pm_colliders.end(), collider);
  pm_colliders.erase(spot);
}

void PhysicsManager::RemoveFromBVH(Node* node)
{
  if (!node || !bvh_root)
  {
    return;
  }

  bvh_root->RemoveFromNode(node);
  numChanges += 1.0f;
  numNodes -= 1.0f;
}

void PhysicsManager::UpdateRoot(Node* node)
{
  bvh_root = node;
}

void PhysicsManager::Update(float dt)
{
  //update bodies - integrates body
  size_t size = pm_bodies.size();
  concurrency::parallel_for(size_t(0), size, [&](size_t i)
    {
      Transform* transform = pm_bodies[i]->GetParent()->GetTransform();

      if (transform->GetScaleChangedStatus())
      {
        pm_bodies[i]->RecalculateInertiaTensor(pm_bodies[i]->GetParent()->GetModel());
      }

      pm_bodies[i]->Update(dt);
    });

  //update transforms - model matrix
  size = pm_transforms.size();
  concurrency::parallel_for(size_t(0), size, [&](size_t i)
    {
      pm_transforms[i]->Update();
    });

  //update colliders - updates collider volumes, updates tree
  size = pm_colliders.size();
  concurrency::parallel_for(size_t(0), size, [&](size_t i)
    {
      if (pm_colliders[i]->GetParent()->GetTransform()->GetHasChangedStatus())
      {
        pm_colliders[i]->Update();
      }
    });

  //draws debug stuff
  if (GraphicsManager::Instance()->GetDebugStatus())
  {
    DrawBVH();
  }

  if ((numChanges / numNodes) > 1.0f)
  {
    numChanges = 0.0f;
    CreateCompactTree();
  }

  //build narrow phase list
  pm_narrowPhase.clear();
  size = pm_colliders.size();

  //concurrency::parallel_for(size_t(0), size, [&](size_t i)
  //  {
  //    if (pm_colliders[i]->GetParent()->GetBody()->b_mass)
  //    {
  //      pm_colliders[i]->CheckCollisionInTree(bvh_root);
  //    }
  //  });

  for (size_t i = 0; i < size; i++)
  {
    if (pm_colliders[i]->GetParent()->GetBody()->b_mass)
    {
      pm_colliders[i]->CheckCollisionInTree(bvh_root);
    }
  }


  //build colliding list
  pm_colliding.clear();
  size = pm_narrowPhase.size();

  concurrency::parallel_for(size_t(0), size, [&](size_t i)
    {
      if (pm_narrowPhase[i].first && pm_narrowPhase[i].second)
      {
        if (pm_narrowPhase[i].first->CheckCollisionWith(pm_narrowPhase[i].second))
        {
          AddCollidingPair(pm_narrowPhase[i].first, pm_narrowPhase[i].second);
        }
      }
    });

  //for (size_t i = 0; i < size; i++)
  //{
  //  if (pm_narrowPhase[i].first->CheckCollisionWith(pm_narrowPhase[i].second))
  //  {
  //    AddCollidingPair(pm_narrowPhase[i].first, pm_narrowPhase[i].second);
  //  }
  //}

  //resolve collision
  size = pm_colliding.size();

  concurrency::parallel_for(size_t(0), size, [&](size_t i)
    {
      if (pm_colliding[i].first && pm_colliding[i].second)
      {
        ResolveCollision(pm_colliding[i].first, pm_colliding[i].second);
      }
    });

  //for (size_t i = 0; i < size; i++)
  //{
  //  ResolveCollision(pm_colliding[i].first, pm_colliding[i].second);
  //}
}

void PhysicsManager::BuildDetectionMap()
{
  int sphere = int(ColliderType::Sphere);

  pm_collisionChekFuncs[sphere][sphere] = CheckSphere;
}

ContactManifold& PhysicsManager::GetContactManifold(Collider* index1, Collider* index2)
{
  return pm_collisionContacts[index1][index2];
}

void PhysicsManager::AddContactManifold(Collider* index1, Collider* index2, const ContactManifold& contact)
{
  tempLock.lock();
  pm_collisionContacts[index1][index2] = contact;
  tempLock.unlock();
}

void PhysicsManager::DrawBVH()
{
  if (bvh_root)
  {
    bvh_root->DrawDebug(Colours::Red);
  }
}

void PhysicsManager::CreateCompactTree()
{
  std::vector<Collider*> current = pm_colliders;
  size_t size = pm_colliders.size();

  //clear tree
  for (size_t i = 0; i < size; i++)
  {
    RemoveFromBVH(pm_colliders[i]->GetNode());
  }

  numNodes = float(size);

  bvh_root = nullptr;

  int currentSize = int(current.size());

  //get pair from with first element
  while (currentSize > 0)
  {
    std::vector<Collider*>::iterator first = current.begin();
    std::vector<Collider*>::iterator closest = current.begin();

    Transform* transform1 = (*first)->GetParent()->GetTransform();
    const vec3& position1 = transform1->GetPosition();

    float closestLen = float(INT_MAX);

    auto curr = first + 1;
    auto end = current.end();

    //find closest node to first node
    while (curr != end)
    {
      Transform* transform2 = (*curr)->GetParent()->GetTransform();
      const vec3& position2 = transform2->GetPosition();

      float dist = glm::distance2(position1, position2);

      if (dist < closestLen)
      {
        closestLen = dist;
        closest = curr;
      }

      ++curr;
    }

    //if not the same element then add to other to create parent then add to tree
    if (closest != first)
    {
      //add closest node to first node
      (*first)->GetNode()->AddToNode((*closest)->GetNode(), false);
      AddToCompactTree((*first)->GetNode()->n_parent);

      Collider* close = *closest;
      current.erase(first);

      //after erasing the first element, the iterator value need to change
      auto deleteIt = current.begin();
      auto end = current.end();

      while (deleteIt != end)
      {
        if ((*deleteIt) == close)
        {
          break;
        }

        ++deleteIt;
      }

      current.erase(deleteIt);
    }
    //only one element in the vector
    else
    {
      AddToCompactTree((*first)->GetNode());
      current.erase(first);
    }

    currentSize -= 2;
  }
}

void PhysicsManager::AddToCompactTree(Node* node)
{
  if (!node)
  {
    return;
  }

  if (!bvh_root)
  {
    bvh_root = node;
    return;
  }

  bvh_root->AddCompactPair(node);
}

void PhysicsManager::AddNarrowPhasePair(Collider* collider1, Collider* collider2)
{
  tempLock.lock();
  pm_narrowPhase.push_back(std::make_pair(collider1, collider2));
  tempLock.unlock();
}

void PhysicsManager::AddCollidingPair(Collider* collider1, Collider* collider2)
{
  tempLock.lock();
  pm_colliding.push_back(std::make_pair(collider1, collider2));
  tempLock.unlock();
}

//TAKE THIS OUT
#include <iostream>
void PhysicsManager::DrawBVHConsole()
{
  DrawBVHConsoleNode(bvh_root, 0);
  std::cout << std::string(100, '-') << std::endl;
}

void PhysicsManager::DrawBVHConsoleNode(Node* node, int space, int level)
{
  const int spacing = 5;
  // Base case  
  if (node == NULL)
  {
    return;
  }

  // Increase distance between levels  
  space += spacing;

  // Process right child first  
  DrawBVHConsoleNode(node->n_right, space, level + 1);

  // Print current node after space  
  // count  
  std::cout << std::endl;
  std::cout << std::string(space - spacing, ' ');

  int balance = node->GetBalance();

  std::cout << "(" << level << ")";
  if (balance < 0)
  {
    if (node->n_collider)
    {
      std::cout << balance << "*" << std::endl;
    }
    else
    {
      std::cout << balance << " " << std::endl;
    }
  }
  else
  {
    if (node->n_collider)
    {
      std::cout << " " << balance << "*" << std::endl;
    }
    else
    {
      std::cout << " " << balance << " " << std::endl;
    }
  }

  // Process left child  
  DrawBVHConsoleNode(node->n_left, space, level + 1);
}

PhysicsManager::PhysicsManager()
  :pm_collisionChekFuncs(int(ColliderType::NumberOfTypes), std::vector<collisionCheck>(int(ColliderType::NumberOfTypes), GJK))
{
  BuildDetectionMap();
}
