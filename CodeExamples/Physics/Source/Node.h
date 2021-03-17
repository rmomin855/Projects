#pragma once
#ifndef NODE
#define NODE

#include <glm/glm.hpp>
#include "GraphicsManager.h"

class Collider;
using namespace glm;

class Node
{
public:
  Node(const vec3& min, const vec3& max, bool leaf = false, Collider* collider = nullptr);

  bool n_leaf;
  Collider* n_collider;

  vec3 n_min;
  vec3 n_max;

  void UpdateNode(float extend = 0.0f);
  void UpdateNode(const vec3& min, const vec3& max);

  bool Contains(Node* node);
  bool Intersects(Node* node);

  void DrawDebug(Colours colour);

  void AddToNode(Node* node, bool replaceRoot = true);
  void AddCompactPair(Node* node);
  bool RemoveFromNode(Node* node);

  int GetBalance();

  Node* n_parent;
  Node* n_left;
  Node* n_right;

private:
  int n_balance;
};

#endif // !NODE
