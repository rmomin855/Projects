/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: NOde.h, decleration for node class (nodes of tree, contain information for bounding volumes)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS350
Author: Rahil Momin, r.momin, 180001717
Creation date: 7-3-20
*/

#pragma once
#ifndef NODE
#define NODE

#include <glm/glm.hpp>
#include <vector>

enum class Colours
{
  Red = 0,
  Blue,
  Green,
  White,
  Yellow,
  Purple,
  Orange,

  NumColours
};

class Mesh;
using namespace glm;

class Node
{
public:
  Node(const vec3& min, const vec3& max, bool leaf = false, Mesh* object = nullptr);
  ~Node();

  bool n_leaf;
  Mesh* n_object;

  vec3 n_min;
  vec3 n_max;

  vec3 n_center;
  float n_radius;

  vec3 n_obbCenter;
  vec3 n_obbScale;
  mat4 n_rotations;

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
