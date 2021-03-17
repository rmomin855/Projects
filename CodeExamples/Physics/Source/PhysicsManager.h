#pragma once
#include <vector>
#include <mutex>

#include "Object.h"

#define EXTEND 30.0f
//#define EXTEND 0.0f

typedef bool (*collisionCheck)(Collider*, Collider*);


struct ContactManifold
{
  glm::vec3 cp_penVec;
  glm::vec3 cp_tangent1;
  glm::vec3 cp_tangent2;

  float depth;

  glm::vec3 cp_World1;
  glm::vec3 cp_World2;
  glm::vec3 cp_Local1;
  glm::vec3 cp_Local2;
};

class PhysicsManager
{
public:
  static PhysicsManager* Instance()
  {
    if (!pm_singleton)
    {
      pm_singleton = new PhysicsManager();
    }

    return pm_singleton;
  }
  static void DeleteInstance()
  {
    delete pm_singleton;
    pm_singleton = nullptr;
  }

  void AddToList(Transform* transform);
  void AddToList(Body* body);
  void AddToList(Collider* collider);

  void RemoveFromList(Transform* transform);
  void RemoveFromList(Body* body);
  void RemoveFromList(Collider* collider);

  void AddToBVH(Node* node);
  void RemoveFromBVH(Node* node);
  void UpdateRoot(Node* node);
  void DrawBVH();

  void CreateCompactTree();
  void AddToCompactTree(Node* node);

  void AddNarrowPhasePair(Collider* collider1, Collider* collider2);
  void AddCollidingPair(Collider* collider1, Collider* collider2);

  //TAKE THIS OUT
  void DrawBVHConsole();
  void DrawBVHConsoleNode(Node* node, int space, int level = 0);

  void Update(float dt);

  void BuildDetectionMap();
  void AddContactManifold(Collider* index1, Collider* index2, const ContactManifold& contact);
  ContactManifold& GetContactManifold(Collider* index1, Collider* index2);

  static float pm_dragConstant;
  static float pm_angularDragConstant;
  static vec3 pm_gravityForce;

  std::mutex BVHmutex;
  float numChanges = 0.0f;

  friend class Body;
  friend class Collider;
private:
  static PhysicsManager* pm_singleton;
  PhysicsManager();
  ~PhysicsManager() = default;

  std::vector<Transform*> pm_transforms;
  std::vector<Body*> pm_bodies;
  std::vector<Collider*> pm_colliders;

  std::vector<std::vector<collisionCheck>> pm_collisionChekFuncs;
  std::unordered_map<Collider*, std::unordered_map<Collider*, ContactManifold>> pm_collisionContacts;

  std::vector<std::pair<Collider*, Collider*>> pm_narrowPhase;
  std::vector<std::pair<Collider*, Collider*>> pm_colliding;

  Node* bvh_root;
  std::mutex tempLock;

  float numNodes = 0.0f;
};

