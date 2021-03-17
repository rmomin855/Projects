/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: BVHTree.h, decleration for BVHTree class (maintains BVH and calculates bounding volumes)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS350
Author: Rahil Momin, r.momin, 180001717
Creation date: 7-3-20
*/

#ifndef BVHTREE
#define BVHTREE

class Node;
class Mesh;
enum class Colours;

#include <vector>
#include <glm/glm.hpp>
#include <map>

enum class BVHType
{
  BottomUP = 0,
  TopDown
};

enum class VolumeType
{
  AABB = 0,
  Sphere,
  OBB,
  Ellipsoid
};

enum class Axis
{
  X = 0,
  Y,
  Z,
  numAxis
};

enum class CutOff
{
  Height = 0,
  Vertices
};

#pragma once
class BVHTree
{
public:
  static BVHTree* Instance()
  {
    if (!tree)
    {
      tree = new BVHTree();
    }

    return tree;
  }

  ~BVHTree();

  void DrawBVH();
  void UpdateRoot(Node* node);

  void BuildTreeBottomUp(std::vector<Mesh*>& meshes);
  void BuildTreeTopDown(std::vector<glm::vec3>& vertexPool, std::vector<glm::vec3>& normalPool,
    int level = 1, Axis axis = Axis::X, Node * parent = nullptr, bool leftChild = true);

  void AddDebugBox(Colours colour, const glm::vec3& pos, const glm::vec3& scale, const glm::mat4& rotation = glm::mat4(1.0f));
  void AddDebugSphere(Colours colour, glm::vec3 pos, float radius);

  void AddToTree(Node* node);
  void AddPairs(Node* node);
  void RemoveFromTree(Node* node);

  void CentroidMethod();
  void RittersMethod();
  void ModLarssonMethod();
  void PCAMethod();

  void CalcualteOBBs();

  void DrawDebug();

  void ChangeTreeType(BVHType changeTo);
  void RebuildTreeTopDown();
  void ClearTree();

  std::map<Colours, std::vector<glm::mat4>> volumes;

  Node* root = nullptr;
  BVHType type = BVHType::BottomUP;
  VolumeType volume = VolumeType::AABB;
  CutOff cutoff = CutOff::Height;

  int maxlevels = 7;
  size_t verticesCutOff = 500;

  std::vector<Node*> nodes;

private:
  static BVHTree* tree;
  BVHTree() = default;
};


#endif // !BVHTREE
