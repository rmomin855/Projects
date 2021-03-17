#ifndef OBJECT
#define OBJECT

#pragma once
#include "Transform.h"
#include "Model.h"
#include "Body.h"
#include "Collider.h"

#include <string>
#include <vector>

typedef void(*UpdateFunc)(float dt);

class Object
{
public:
  Object(Transform* transform = nullptr);
  Object(const char* name, Transform* transform = nullptr);
  ~Object();

  void SetName(const char* name);
  void SetTransform(Transform* transform);
  void SetModel(Model* model);
  void SetBody(Body* body);
  void SetCollider(Collider* collider);

  std::string& GetName();
  Transform* GetTransform();
  Model* GetModel();
  Body* GetBody();
  Collider* GetCollider();

  UpdateFunc update;
private:
  std::string obj_name;

  Transform* obj_transform;
  Model* obj_model;
  Body* obj_body;
  Collider* obj_collider;
};

#endif // !OBJECT
