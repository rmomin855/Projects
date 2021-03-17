#ifndef GRAPHICS
#define GRAPHICS

#pragma once
#include "Model.h"

#include <glm/glm.hpp>
#include <vector>
#include "Camera.h"
#include <map>
#include <mutex>

using namespace glm;

enum class Colours
{
  Red = 0,
  Blue,
  White,
  Green,
  Yellow,
  Purple,
  Orange,
  Black,

  NumColours
};

class GraphicsManager
{
public:
  static GraphicsManager* Instance()
  {
    if (!gm_singleton)
    {
      gm_singleton = new GraphicsManager();
    }

    return gm_singleton;
  }
  static void DeleteInstance()
  {
    delete gm_singleton;
    gm_singleton = nullptr;
  }

  void AddToList(Model* model);
  void RemoveFromList(Model* model);

  void DrawDebugLine(const vec3& p1, const vec3& p2, Colours colour = Colours::White);
  void DrawDebugPoint(const vec3& point, Colours colour = Colours::White);
  void DrawDebugBox(Colours colour, const mat4& modelMatrix);
  void DrawDebugBox(Colours colour, const vec3& pos, const vec3& scale = vec3(1.0f), const vec3& rotation = vec3());

  void Render(Camera* camera);
  void FlushDebug();

  void DrawDebug(bool debug);
  bool GetDebugStatus();

  void UpdateFOV(float fov);

  bool gm_drawDebug;
  bool gm_drawLines = true;
  bool gm_drawPoints = true;
  bool gm_drawBoxes = true;

private:
  static GraphicsManager* gm_singleton;
  GraphicsManager();
  ~GraphicsManager();

  std::vector<Model*> gm_models;
  float gm_fov;
  float gm_near;
  float gm_far;
  mat4 gm_projMat;

  void RenderDebug(Camera* camera);
  void RenderDebugLines();
  void RenderDebugPoints();
  void RenderDebugBoxes();

  Mesh* gm_debugMesh;
  GLuint gm_debugProgram;
  std::map<Colours, std::vector<vec3>> gm_debugLines;
  std::map<Colours, std::vector<vec3>> gm_debugPoints;
  GLuint gm_debugBufferSize;
  std::map<Colours, std::vector<mat4>> gm_debugBoxes;

  std::mutex lineMutex;
  std::mutex pointMutex;
  std::mutex boxMutex;
};

#endif // !GRAPHICS
