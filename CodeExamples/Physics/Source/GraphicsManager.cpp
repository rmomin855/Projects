#include "GraphicsManager.h"
#include <algorithm>
#include "Object.h"
#include "ShaderManager.h"
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

GraphicsManager* GraphicsManager::gm_singleton = nullptr;

vec3 GetColour(Colours colour)
{
  switch (colour)
  {
  case Colours::Red:
    return vec3(1.0f, 0.0f, 0.0f);
  case Colours::Blue:
    return vec3(0.0f, 0.0f, 1.0f);
  case Colours::White:
    return vec3(1.0f, 1.0f, 1.0f);
  case Colours::Green:
    return vec3(0.0f, 1.0f, 0.0f);
  case Colours::Purple:
    return vec3(0.4f, 0.0f, 0.8f);
  case Colours::Orange:
    return vec3(1.0f, 0.55f, 0.0f);
  case Colours::Yellow:
    return vec3(1.0f, 1.0f, 0.0f);
  case Colours::Black:
    return vec3(0.0f, 0.0f, 0.0f);
  }

  return vec3();
}

void GraphicsManager::AddToList(Model* model)
{
  gm_models.push_back(model);
}

void GraphicsManager::RemoveFromList(Model* model)
{
  auto spot = std::find(gm_models.begin(), gm_models.end(), model);
  gm_models.erase(spot);
}

void GraphicsManager::DrawDebugLine(const vec3& p1, const vec3& p2, Colours colour)
{
  if (!gm_drawLines)
  {
    return;
  }

  lineMutex.lock();
  gm_debugLines[colour].push_back(p1);
  gm_debugLines[colour].push_back(p2);
  lineMutex.unlock();
}

void GraphicsManager::DrawDebugPoint(const vec3& point, Colours colour)
{
  if (!gm_drawPoints)
  {
    return;
  }

  pointMutex.lock();
  gm_debugPoints[colour].push_back(point);
  pointMutex.unlock();
}

void GraphicsManager::DrawDebugBox(Colours colour, const mat4& modelMatrix)
{
  if (!gm_drawBoxes)
  {
    return;
  }

  boxMutex.lock();
  gm_debugBoxes[colour].push_back(modelMatrix);
  boxMutex.unlock();
}

void GraphicsManager::DrawDebugBox(Colours colour, const vec3& pos, const vec3& scale, const vec3& rotation)
{
  if (!gm_drawBoxes)
  {
    return;
  }

  mat4 matrix(1.0f);
  matrix = glm::translate(matrix, pos);
  matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  matrix = glm::scale(matrix, scale);

  boxMutex.lock();
  gm_debugBoxes[colour].push_back(matrix);
  boxMutex.unlock();
}

void GraphicsManager::Render(Camera* camera)
{
  camera->Update();

  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (Model* model : gm_models)
  {
    glUseProgram(model->m_program);

    glBindVertexArray(model->m_mesh->VAO);

    GLint vTransformLoc = glGetUniformLocation(model->m_program, "ModelTransform");
    if (vTransformLoc >= 0)
    {
      glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &(model->GetParent()->GetTransform()->GetModelMatrix()[0][0]));
    }

    vTransformLoc = glGetUniformLocation(model->m_program, "ViewTransform");
    if (vTransformLoc >= 0)
    {
      glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &camera->GetViewMatrix()[0][0]);
    }

    vTransformLoc = glGetUniformLocation(model->m_program, "ProjTransform");
    if (vTransformLoc >= 0)
    {
      glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &gm_projMat[0][0]);
    }

    vTransformLoc = glGetUniformLocation(model->m_program, "colour");
    if (vTransformLoc >= 0)
    {
      glUniform3fv(vTransformLoc, 1, &model->m_colour[0]);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, model->m_mesh->NumberOfElements, GL_UNSIGNED_INT, 0);
  }

  if (gm_drawDebug)
  {
    RenderDebug(camera);
  }
}

void GraphicsManager::RenderDebug(Camera* camera)
{
  RenderDebugBoxes();

  glClear(GL_DEPTH_BUFFER_BIT);

  glUseProgram(gm_debugProgram);
  glBindVertexArray(gm_debugMesh->VAO);

  GLint vTransformLoc = glGetUniformLocation(gm_debugProgram, "ViewTransform");
  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &camera->GetViewMatrix()[0][0]);
  }

  vTransformLoc = glGetUniformLocation(gm_debugProgram, "ProjTransform");
  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &gm_projMat[0][0]);
  }

  RenderDebugLines();
  RenderDebugPoints();
}

void GraphicsManager::RenderDebugLines()
{
  if (!gm_drawLines)
  {
    return;
  }

  auto currLine = gm_debugLines.begin();
  auto endLine = gm_debugLines.end();

  while (currLine != endLine)
  {
    if (currLine->second.size() > gm_debugBufferSize)
    {
      glNamedBufferStorage(gm_debugMesh->VBO, sizeof(vec3) * currLine->second.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    glNamedBufferSubData(gm_debugMesh->VBO, 0, sizeof(vec3) * currLine->second.size(), currLine->second.data());

    GLint vTransformLoc = glGetUniformLocation(gm_debugProgram, "colour");
    if (vTransformLoc >= 0)
    {
      glUniform3fv(vTransformLoc, 1, &GetColour(currLine->first)[0]);
    }

    glLineWidth(5);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_LINES, 0, currLine->second.size());

    currLine++;
  }
}

void GraphicsManager::RenderDebugPoints()
{
  if (!gm_drawPoints)
  {
    return;
  }

  auto currPoint = gm_debugPoints.begin();
  auto endPoint = gm_debugPoints.end();

  while (currPoint != endPoint)
  {
    if (currPoint->second.size() % 2)
    {
      currPoint->second.push_back(currPoint->second.back());
    }

    if (currPoint->second.size() > gm_debugBufferSize)
    {
      glNamedBufferStorage(gm_debugMesh->VBO, sizeof(vec3) * currPoint->second.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    glNamedBufferSubData(gm_debugMesh->VBO, 0, sizeof(vec3) * currPoint->second.size(), currPoint->second.data());

    GLint vTransformLoc = glGetUniformLocation(gm_debugProgram, "colour");
    if (vTransformLoc >= 0)
    {
      glUniform3fv(vTransformLoc, 1, &GetColour(currPoint->first)[0]);
    }

    glPointSize(10);
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glDrawArrays(GL_POINTS, 0, currPoint->second.size());

    currPoint++;
  }
}

void GraphicsManager::RenderDebugBoxes()
{
  if (!gm_drawBoxes)
  {
    return;
  }

  GLuint program = ShaderManager::Instance()->GetShader("Simple");
  Mesh* mesh = MeshManager::Instance()->GetMesh("cube2");

  glUseProgram(program);
  glBindVertexArray(mesh->VAO);

  auto curr = gm_debugBoxes.begin();
  auto end = gm_debugBoxes.end();

  while (curr != end)
  {
    vec3 colour = GetColour(curr->first);

    for (mat4& model : curr->second)
    {
      GLint vTransformLoc = glGetUniformLocation(program, "ModelTransform");
      if (vTransformLoc >= 0)
      {
        glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &model[0][0]);
      }

      vTransformLoc = glGetUniformLocation(program, "colour");
      if (vTransformLoc >= 0)
      {
        glUniform3fv(vTransformLoc, 1, &colour[0]);
      }

      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDrawElements(GL_TRIANGLES, mesh->NumberOfElements, GL_UNSIGNED_INT, 0);
    }

    ++curr;
  }
}

void GraphicsManager::FlushDebug()
{
  gm_debugLines.clear();
  gm_debugPoints.clear();
  gm_debugBoxes.clear();
}

void GraphicsManager::DrawDebug(bool debug)
{
  gm_drawDebug = debug;
}

bool GraphicsManager::GetDebugStatus()
{
  return gm_drawDebug;
}

void GraphicsManager::UpdateFOV(float fov)
{
  gm_fov += fov;

  gm_fov = gm_fov < 1.0f ? 1.0f : gm_fov;
  gm_fov = gm_fov > 45.0f ? 45.0f : gm_fov;

  gm_projMat = glm::perspective(glm::radians(gm_fov), 16.0f / 9.0f, gm_near, gm_far);
}

GraphicsManager::GraphicsManager()
  :gm_models(), gm_debugMesh(), gm_debugLines(), gm_debugPoints(), gm_debugBufferSize(1000), gm_projMat(),
  gm_drawDebug(true), gm_fov(45.0f), gm_near(1.0f), gm_far(100000.0f)
{
  gm_debugMesh = new Mesh();
  size_t bufferSize = 1000 * sizeof(vec3);

  glCreateBuffers(1, &gm_debugMesh->VBO); // glGenBuffers + glBindBuffer

  glNamedBufferStorage(gm_debugMesh->VBO, // same as glBufferData
    bufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);

  glCreateVertexArrays(1, &gm_debugMesh->VAO); // glGenVertexArrays + glBindVertexArray

  glVertexArrayVertexBuffer(gm_debugMesh->VAO, 0, gm_debugMesh->VBO, 0, sizeof(glm::vec3));

  glEnableVertexArrayAttrib(gm_debugMesh->VAO, 0); // enable index 0: pos attribute

  // data format for vertex position attributes
  glVertexArrayAttribFormat(gm_debugMesh->VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

  // associate VAO binding 0 to vertex attribute index 0
  glVertexArrayAttribBinding(gm_debugMesh->VAO, 0, 0);

  gm_debugProgram = ShaderManager::Instance()->GetShader("Debug");

  gm_projMat = glm::perspective(glm::radians(gm_fov), 16.0f / 9.0f, gm_near, gm_far);
}

GraphicsManager::~GraphicsManager()
{
  delete gm_debugMesh;
}
