#include "Node.h"
#include "Collider.h"
#include "PhysicsManager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

Node::Node(const vec3& min, const vec3& max, bool leaf, Collider* collider)
  : n_min(min), n_max(max), n_leaf(leaf), n_collider(collider),
  n_left(), n_right(), n_parent(), n_balance()
{
  if (n_collider)
  {
    n_collider->SetNode(this);
  }
}

void Node::UpdateNode(float extend)
{
  //minimum of both nodes would be compact min
  n_min = glm::min(n_left->n_min, n_right->n_min);
  //maximum of both nodes would be compact max
  n_max = glm::max(n_left->n_max, n_right->n_max);

  if (n_left->n_leaf || n_right->n_leaf)
  {
    n_min -= extend;
    n_max += extend;
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
}

bool Node::Contains(Node* node)
{
  if ((node->n_min.x < n_min.x) || (node->n_min.y < n_min.y) || (node->n_min.z < n_min.z))
  {
    return false;
  }

  if ((node->n_max.x > n_max.x) || (node->n_max.y > n_max.y) || (node->n_max.z > n_max.z))
  {
    return false;
  }

  return true;
}

bool Node::Intersects(Node* node)
{
  if (n_min.x > node->n_max.x || n_min.y > node->n_max.y || n_min.z > node->n_max.z)
  {
    return false;
  }

  if (node->n_min.x > n_max.x || node->n_min.y > n_max.y || node->n_min.z > n_max.z)
  {
    return false;
  }

  return true;
}

void Node::DrawDebug(Colours colour)
{
  vec3 pos = (n_min + n_max) / 2.0f;
  vec3 scale = (n_max - n_min) / 2.0f;
  GraphicsManager::Instance()->DrawDebugBox(colour, pos, scale);

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
      PhysicsManager::Instance()->UpdateRoot(newNode);
    }

    //new node parent is now this nodes parent
    newNode->n_parent = n_parent;

    //node and this node are now newNode's children
    newNode->n_left = this;
    n_parent = newNode;

    newNode->n_right = node;
    node->n_parent = newNode;

    return newNode->UpdateNode(EXTEND);
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
  if (n_left->n_leaf || n_right->n_leaf)
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
      PhysicsManager::Instance()->UpdateRoot(newNode);
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
        PhysicsManager::Instance()->UpdateRoot(nullptr);
        return true;
      }

      //if this is left child then get right child or get left child
      Node* otherChild = nullptr;
      otherChild = n_parent->n_left == this ? n_parent->n_right : n_parent->n_left;

      //lol
      Node* Grandparent = n_parent->n_parent;

      //since only one leaf node, no need for weird useless parent
      otherChild->n_parent = Grandparent;

      //if parent has a parent, update its pointer
      if (Grandparent)
      {
        Grandparent->n_left = Grandparent->n_left == n_parent ? otherChild : Grandparent->n_left;
        Grandparent->n_right = Grandparent->n_right == n_parent ? otherChild : Grandparent->n_right;
      }
      //if no parent's parent then parent was root
      else
      {
        PhysicsManager::Instance()->UpdateRoot(otherChild);
      }

      //delete parent
      delete n_parent;
      n_parent = nullptr;

      if (otherChild->n_parent)
      {
        otherChild->n_parent->UpdateNode(EXTEND);
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
