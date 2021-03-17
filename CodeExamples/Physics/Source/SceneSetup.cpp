#include "SceneSetup.h"
#include "Object.h"
#include "ShaderManager.h"
#include "MeshManager.h"
#include "GraphicsManager.h"
#include "Generic.h"

#include <time.h>
#include <GLFW/glfw3.h>
#include "System.h"

#include "CollisionHelperFunctions.h"


#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

std::vector<Object*> objects;
std::vector<vec3> colours;

void SceneSetup::Load()
{
  srand(size_t(time(NULL)));

  System::Instance()->GetCamera()->SetPosition(vec3(0.0f, 50.0f, 150.0f));
}

void SceneSetup::Init()
{
  Object* tempObject = new Object("Static");

  Transform* transform = new Transform(vec3(0.0f), vec3(1000.0f, 10.0f, 1000.0f));
  Model* model = new Model(ShaderManager::Instance()->GetShader("Simple"), MeshManager::Instance()->GetMesh("cube2"), vec3(1.0f));
  Body* body = new Body(transform, model, 0.0f);
  Generic* genericCollider = new Generic(transform, vec3(1.0f));

  tempObject->SetTransform(transform);
  tempObject->SetModel(model);
  tempObject->SetBody(body);
  tempObject->SetCollider(genericCollider);

  colours.push_back(vec3(1.0f));
  objects.push_back(tempObject);

  tempObject = new Object("NonStatic");

  float x = 0.0f;
  float y = 20.0f;
  float z = 0.0f;

  float Rx = (float(rand()) / float(RAND_MAX)) * 180.0f - 90.0f;
  float Ry = (float(rand()) / float(RAND_MAX)) * 180.0f - 90.0f;
  float Rz = (float(rand()) / float(RAND_MAX)) * 180.0f - 90.0f;

  float Cx = (float(rand()) / float(RAND_MAX));
  float Cz = (float(rand()) / float(RAND_MAX));
  vec3 colour = vec3(Cx, 0.0f, Cz);
  colours.push_back(colour);

  transform = new Transform(vec3(x, y, z), vec3(5.0f), vec3(Rx, Ry, Rz));
  model = new Model(ShaderManager::Instance()->GetShader("Simple"), MeshManager::Instance()->GetMesh("cube2"), colour);
  body = new Body(transform, model, 2.0f);
  genericCollider = new Generic(transform, vec3(1.0f));

  tempObject->SetTransform(transform);
  tempObject->SetModel(model);
  tempObject->SetBody(body);
  tempObject->SetCollider(genericCollider);

  objects.push_back(tempObject);

  ///////////////////////////////////////////////////////////////////////////////////////
}

void SceneSetup::Update(float dt)
{
  for (size_t i = 0; i < objects.size(); i++)
  {
    vec3 pos = objects[i]->GetTransform()->GetPosition();
    if (pos.y < 0.0f)
    {
      SceneManager::Instance()->ResetState();
    }
  }
}

void SceneSetup::DeInit()
{
  size_t size = objects.size();
  for (size_t i = 0; i < size; i++)
  {
    delete objects[i];
  }

  objects.clear();
}

void SceneSetup::UnLoad()
{
}