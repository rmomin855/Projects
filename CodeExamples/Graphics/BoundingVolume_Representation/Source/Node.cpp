/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Node.cpp, implementation for node class (nodes of tree, contain information for bounding volumes)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS350
Author: Rahil Momin, r.momin, 180001717
Creation date: 7-3-20
*/

#include "Node.h"
#include "Object.h"
#include "BVHTree.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include "SimpleScene_Quad.h"
extern Scene* scene;

bool DrawObjectNodes = true;
bool DrawBVH = true;

Node::Node(const vec3& min, const vec3& max, bool leaf, Mesh* object)
  : n_min(min), n_max(max), n_leaf(leaf),
  n_left(), n_right(), n_parent(), n_balance(),
  n_center((n_min + n_max) * 0.5f), n_radius(glm::length((n_max - n_min) * 0.5f)), n_object(object),
  n_rotations(), n_obbCenter()
{
  if (object)
  {
    object->node = this;
  }
}

Node::~Node()
{
  if (n_left)
  {
    delete n_left;
  }
  if (n_right)
  {
    delete n_right;
  }
  if (n_object)
  {
    delete n_object;
  }
}

void Node::UpdateNode(float extend)
{
  if (n_left && n_right)
  {
    //minimum of both nodes would be compact min
    n_min = glm::min(n_left->n_min, n_right->n_min);
    //maximum of both nodes would be compact max
    n_max = glm::max(n_left->n_max, n_right->n_max);

    n_center = (n_left->n_center + n_right->n_center) * 0.5f;

    float right = length(n_right->n_center - n_center) + n_right->n_radius;
    float left = length(n_left->n_center - n_center) + n_left->n_radius;

    n_radius = left < right ? right : left;
  }

  if (n_parent)
  {
    n_parent->UpdateNode(extend);
  }
}

void Node::UpdateNode(const vec3& min, const vec3& max)
{
  n_min = min;
  n_max = max;

  n_center = (n_left->n_center + n_right->n_center) * 0.5f;

  float right = length(n_right->n_center - n_center) + n_right->n_radius;
  float left = length(n_left->n_center - n_center) + n_left->n_radius;

  n_radius = n_radius < right ? right : n_radius;
  n_radius = n_radius < left ? left : n_radius;
}

bool Node::Contains(Node* node)
{
  if (BVHTree::Instance()->volume == VolumeType::AABB)
  {
    if ((node->n_min.x < n_min.x) || (node->n_min.y < n_min.y) || (node->n_min.z < n_min.z))
    {
      return false;
    }

    if ((node->n_max.x > n_max.x) || (node->n_max.y > n_max.y) || (node->n_max.z > n_max.z))
    {
      return false;
    }
  }
  else
  {
    float dist = glm::distance2(n_center, node->n_center);

    if (dist > powf(n_radius - node->n_radius, 2.0f))
    {
      return false;
    }
  }

  return true;
}

bool Node::Intersects(Node* node)
{
  if (BVHTree::Instance()->volume == VolumeType::AABB)
  {
    if (n_min.x > node->n_max.x || n_min.y > node->n_max.y || n_min.z > node->n_max.z)
    {
      return false;
    }

    if (node->n_min.x > n_max.x || node->n_min.y > n_max.y || node->n_min.z > n_max.z)
    {
      return false;
    }
  }
  else
  {
    float dist = glm::distance2(n_center, node->n_center);

    if (dist > powf(n_radius + node->n_radius, 2.0f))
    {
      return false;
    }
  }

  return true;
}

void Node::DrawDebug(Colours colour)
{
  vec3 pos = (n_max + n_min) / 2.0f;
  vec3 scale = (n_max - n_min) / 2.0f;

  int next = int(colour) + 1;
  next = next == int(Colours::NumColours) ? 0 : next;

  if (n_left)
  {
    n_left->DrawDebug(Colours(next));
  }
  if (n_right)
  {
    n_right->DrawDebug(Colours(next));
  }

  if (n_leaf && !DrawObjectNodes)
  {
    return;
  }

  if (!n_leaf && !DrawBVH)
  {
    return;
  }

  VolumeType volume = BVHTree::Instance()->volume;
  if (volume == VolumeType::AABB || (volume == VolumeType::OBB && !n_leaf))
  {
    BVHTree::Instance()->AddDebugBox(colour, pos, scale);
  }
  else if (volume == VolumeType::OBB)
  {
    BVHTree::Instance()->AddDebugBox(colour, n_obbCenter, n_obbScale, n_rotations);
  }
  else if (volume == VolumeType::Sphere || (volume == VolumeType::Ellipsoid && !n_leaf))
  {
    BVHTree::Instance()->AddDebugSphere(colour, n_center, n_radius);
  }
  else if (volume == VolumeType::Ellipsoid)
  {
    BVHTree::Instance()->AddDebugBox(colour, n_obbCenter, n_obbScale * 1.5f, n_rotations);
  }
}

void Node::AddToNode(Node* node, bool replaceRoot)
{
  //if last level in the tree
  if (n_leaf)
  {
    //create new node sice the node cannot be a child node of a leaf
    Node* newNode = new Node(vec3(-1.0f), vec3(1.0f));

    //update parent node to point at new node
    if (n_parent)
    {
      n_parent->n_left = n_parent->n_left == this ? newNode : n_parent->n_left;
      n_parent->n_right = n_parent->n_right == this ? newNode : n_parent->n_right;
    }
    //only root will not have a parent
    else if (replaceRoot)
    {
      BVHTree::Instance()->UpdateRoot(newNode);
    }

    //new node parent is now this nodes parent
    newNode->n_parent = n_parent;

    //node and this node are now newNode's children
    newNode->n_left = this;
    n_parent = newNode;

    newNode->n_right = node;
    node->n_parent = newNode;

    return newNode->UpdateNode();
  }

  //SHOULD I DO THIS INSTEAD?
  if (n_balance < 0)
  {
    ++n_balance;
    return n_left->AddToNode(node);
  }
  else
  {
    --n_balance;
    return n_right->AddToNode(node);
  }
}

void Node::AddCompactPair(Node* node)
{
  if (n_left->n_leaf && n_right->n_leaf)
  {
    //create new node sice the node cannot be a child node of a leaf
    Node* newNode = new Node(vec3(-1.0f), vec3(1.0f));

    //update parent node to point at new node
    if (n_parent)
    {
      n_parent->n_left = n_parent->n_left == this ? newNode : n_parent->n_left;
      n_parent->n_right = n_parent->n_right == this ? newNode : n_parent->n_right;
    }
    //only root will not have a parent
    else
    {
      BVHTree::Instance()->UpdateRoot(newNode);
    }

    //new node parent is now this nodes parent
    newNode->n_parent = n_parent;

    //node and this node are now newNode's children
    newNode->n_left = this;
    n_parent = newNode;

    newNode->n_right = node;
    node->n_parent = newNode;

    return newNode->UpdateNode();
  }

  //SHOULD I DO THIS INSTEAD?
  if (n_balance < 0)
  {
    ++n_balance;
    return n_left->AddCompactPair(node);
  }
  else
  {
    --n_balance;
    return n_right->AddCompactPair(node);
  }
}

bool Node::RemoveFromNode(Node* node)
{
  if (n_leaf)
  {
    if (this == node)
    {
      //last node(root)
      if (!n_parent)
      {
        BVHTree::Instance()->UpdateRoot(nullptr);
        return true;
      }

      //if this is left child then get right child or get left child
      Node* otherChild = nullptr;
      otherChild = n_parent->n_left == this ? n_parent->n_right : n_parent->n_left;

      //lol
      Node* Grandparent = n_parent->n_parent;

      if (otherChild)
      {
        //since only one leaf node, no need for weird useless parent
        otherChild->n_parent = Grandparent;
      }

      //if parent has a parent, update its pointer
      if (Grandparent)
      {
        Grandparent->n_left = Grandparent->n_left == n_parent ? otherChild : Grandparent->n_left;
        Grandparent->n_right = Grandparent->n_right == n_parent ? otherChild : Grandparent->n_right;
      }
      //if no parent's parent then parent was root
      else
      {
        BVHTree::Instance()->UpdateRoot(otherChild);
      }

      //delete parent
      delete n_parent;
      n_parent = nullptr;

      if (otherChild && otherChild->n_parent)
      {
        otherChild->n_parent->UpdateNode();
      }

      return true;
    }
  }
  else
  {
    //if not a leaf node then go down the tree to 
    //whichever branch that contains the node
    if (n_left && n_left->Intersects(node))
    {
      if (n_left->RemoveFromNode(node))
      {
        --n_balance;
        return true;
      }
    }
    if (n_right && n_right->Intersects(node))
    {
      if (n_right->RemoveFromNode(node))
      {
        ++n_balance;
        return true;
      }
    }
  }

  return false;
}

int Node::GetBalance()
{
  return n_balance;
}
