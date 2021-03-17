#include "Object.h"

Object::Object(Transform* transform)
  : obj_name("John Doe"), obj_transform(transform), obj_model(nullptr), obj_body(nullptr), obj_collider(nullptr), update(nullptr)
{}

Object::Object(const char* name, Transform* transform)
  : obj_name(name), obj_transform(transform), obj_model(nullptr), obj_body(nullptr), obj_collider(nullptr), update(nullptr)
{}

Object::~Object()
{
  if (obj_transform)
  {
    delete obj_transform;
  }
  if (obj_model)
  {
    delete obj_model;
  }
  if (obj_body)
  {
    delete obj_body;
  }
  if (obj_collider)
  {
    delete obj_collider;
  }
}

void Object::SetName(const char* name)
{
  obj_name = name;
}

void Object::SetTransform(Transform* transform)
{
  obj_transform = transform;
  obj_transform->SetParent(this);
}

void Object::SetModel(Model* model)
{
  obj_model = model;
  obj_model->SetParent(this);
}

void Object::SetBody(Body* body)
{
  obj_body = body;
  obj_body->SetParent(this);
}

void Object::SetCollider(Collider* collider)
{
  obj_collider = collider;
  obj_collider->SetParent(this);
}

std::string& Object::GetName()
{
  return obj_name;
}

Transform* Object::GetTransform()
{
  return obj_transform;
}

Model* Object::GetModel()
{
  return obj_model;
}

Body* Object::GetBody()
{
  return obj_body;
}

Collider* Object::GetCollider()
{
  return obj_collider;
}
