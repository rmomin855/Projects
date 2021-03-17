#ifndef COLLIDER
#define COLLIDER

#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Node.h"
#include "Transform.h"

enum class ColliderType
{
  Generic = 0,
  Sphere,

  NumberOfTypes
};

class Object;
class Collider
{
public:
  Collider(Transform* transform, ColliderType type = ColliderType::Generic);
  ~Collider();

  void SetParent(Object* parent);
  void SetNode(Node* node);
  Object* GetParent();
  Node* GetNode();

  void CheckCollisionInTree(Node* node);
  bool CheckCollisionWith(Collider* object2);

  virtual void Update(float dt = 0.0f);

  //members
  ColliderType c_type;

  size_t c_index;
protected:
  Object* c_parent;
  Node* c_node;
};

#endif // !COLLIDER
