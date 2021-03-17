#ifndef INPUTMANAGER
#define INPUTMANAGER

#pragma once
#include <GLFW/glfw3.h>

class System;

class InputManager
{
public:
  static InputManager* Instance()
  {
    if (!im_singleton)
    {
      im_singleton = new InputManager();
    }

    return im_singleton;
  }
  static void DeleteInstance()
  {
    delete im_singleton;
    im_singleton = nullptr;
  }

  void Update(System* system);
  void CameraInput(System* system);

  friend class System;
private:
  static InputManager* im_singleton;
  InputManager() = default;
  ~InputManager() = default;
};

#endif // !INPUTMANAGER
