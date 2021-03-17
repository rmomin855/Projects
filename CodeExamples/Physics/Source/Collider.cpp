#include "Collider.h"
#include "PhysicsManager.h"
#include "Generic.h"

static size_t counter = 0;

Collider::Collider(Transform* transform, ColliderType type)
  : c_type(type), c_parent(), c_node(), c_index(counter++)
{
  const vec3& scale = transform->GetScale();
  const vec3& pos = transform->GetPosition();
  Node* node = new Node(pos - scale, pos + scale, true, this);

  PhysicsManager::Instance()->AddToList(this);
  PhysicsManager::Instance()->AddToBVH(c_node);
}

Collider::~Collider()
{
  PhysicsManager::Instance()->RemoveFromList(this);
  PhysicsManager::Instance()->RemoveFromBVH(c_node);
  delete c_node;
}

void Collider::SetParent(Object* parent)
{
  c_parent = parent;
}

void Collider::SetNode(Node* node)
{
  c_node = node;
}

Object* Collider::GetParent()
{
  return c_parent;
}

Node* Collider::GetNode()
{
  return c_node;
}

void Collider::CheckCollisionInTree(Node* node)
{
  if (c_node->Intersects(node))
  {
    if (node->n_leaf)
    {
      if (c_node == node)
      {
        return;
      }

      PhysicsManager::Instance()->AddNarrowPhasePair(this, node->n_collider);
      return;
    }
    else
    {
      CheckCollisionInTree(node->n_left);
      CheckCollisionInTree(node->n_right);
    }
  }
}

bool Collider::CheckCollisionWith(Collider* object2)
{
  int type1 = int(c_type);
  int type2 = int(object2->c_type);

  return PhysicsManager::Instance()->pm_collisionChekFuncs[type1][type2](this, object2);
}

void Collider::Update(float dt)
{
  Node* parent = c_node->n_parent;

  if (parent && !parent->Contains(c_node))
  {
    PhysicsManager::Instance()->BVHmutex.lock();
    parent->UpdateNode(EXTEND);
    PhysicsManager::Instance()->numChanges++;
    PhysicsManager::Instance()->BVHmutex.unlock();
  }
}
