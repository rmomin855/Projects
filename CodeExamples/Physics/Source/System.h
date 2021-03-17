#ifndef SYSTEM
#define SYSTEM

#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "SceneManager.h"
#include "InputManager.h"
#include "MeshManager.h"
#include "ShaderManager.h"
#include "GraphicsManager.h"
#include "PhysicsManager.h"

#include "Camera.h"

class System
{
public:
  static System* Instance(unsigned int width = 1024, unsigned int height = 768)
  {
    if (!sys_singleton)
    {
      sys_singleton = new System(width, height);
    }

    return sys_singleton;
  }

  static void DeleteInstance()
  {
    delete sys_singleton;
    sys_singleton = nullptr;
  }

  void Init();
  void Update(float dt);
  void cleanup();

  bool Running();
  void Stop();

  GLFWwindow* GetWindow();
  Camera* GetCamera();

  bool sys_paused;
  bool sys_step;

  friend class InputManager;
private:
  static System* sys_singleton;
  System(unsigned int width = 1024, unsigned int height = 768);
  ~System();


  bool sys_isRunning;
  unsigned int sys_screenWidth;
  unsigned int sys_screenHeight;
  GLFWwindow* sys_window;

  SceneManager* sys_sceneManager;
  InputManager* sys_inputManager;
  MeshManager* sys_meshManager;
  ShaderManager* sys_shaderManager;
  GraphicsManager* sys_graphicsManager;
  PhysicsManager* sys_physicsManager;

  Camera* sys_camera;

  std::chrono::time_point<std::chrono::system_clock> start, end;
  int frames = 0;
};

#endif // !SYSTEM
