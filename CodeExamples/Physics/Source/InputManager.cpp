#include "System.h"
#include "Camera.h"

InputManager* InputManager::im_singleton = nullptr;
static bool consoleTree = false;
static bool compactTree = false;
static bool stepOnce = false;

void InputManager::Update(System* system)
{
  if (glfwGetKey(system->sys_window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(system->sys_window))
  {
    system->Stop();
  }

  if (glfwGetKey(system->sys_window, GLFW_KEY_N) == GLFW_PRESS)
  {
    system->sys_paused = true;
  }
  if (glfwGetKey(system->sys_window, GLFW_KEY_M) == GLFW_PRESS)
  {
    system->sys_paused = false;
  }
  if (glfwGetKey(system->sys_window, GLFW_KEY_O) == GLFW_PRESS)
  {
    system->sys_graphicsManager->DrawDebug(false);
  }
  if (glfwGetKey(system->sys_window, GLFW_KEY_I) == GLFW_PRESS)
  {
    system->sys_graphicsManager->DrawDebug(true);
  }

  //step
  if (glfwGetKey(system->sys_window, GLFW_KEY_P) == GLFW_PRESS)
  {
    if (!stepOnce)
    {
      stepOnce = true;
      system->sys_step = !system->sys_step;
    }
  }
  if (glfwGetKey(system->sys_window, GLFW_KEY_P) == GLFW_RELEASE)
  {
    stepOnce = false;
  }

  //draw console
  if (glfwGetKey(system->sys_window, GLFW_KEY_B) == GLFW_PRESS)
  {
    if (!consoleTree)
    {
      PhysicsManager::Instance()->DrawBVHConsole();
      consoleTree = true;
    }
  }
  if (glfwGetKey(system->sys_window, GLFW_KEY_B) == GLFW_RELEASE)
  {
    consoleTree = false;
  }

  //compact tree
  if (glfwGetKey(system->sys_window, GLFW_KEY_V) == GLFW_PRESS)
  {
    if (!compactTree)
    {
      PhysicsManager::Instance()->CreateCompactTree();
      compactTree = true;
    }
  }
  if (glfwGetKey(system->sys_window, GLFW_KEY_V) == GLFW_RELEASE)
  {
    compactTree = false;
  }

  CameraInput(system);
}

void InputManager::CameraInput(System* system)
{
  if (glfwGetKey(system->sys_window, GLFW_KEY_W) == GLFW_PRESS)
  {
    system->sys_camera->cam_position += system->sys_camera->cam_speed * system->sys_camera->cam_front;
    system->sys_camera->cam_dirty = true;
  }
  if (glfwGetKey(system->sys_window, GLFW_KEY_S) == GLFW_PRESS)
  {
    system->sys_camera->cam_position -= system->sys_camera->cam_speed * system->sys_camera->cam_front;
    system->sys_camera->cam_dirty = true;
  }
  if (glfwGetKey(system->sys_window, GLFW_KEY_A) == GLFW_PRESS)
  {
    system->sys_camera->cam_position += system->sys_camera->cam_speed * system->sys_camera->cam_right;
    system->sys_camera->cam_dirty = true;
  }
  if (glfwGetKey(system->sys_window, GLFW_KEY_D) == GLFW_PRESS)
  {
    system->sys_camera->cam_position -= system->sys_camera->cam_speed * system->sys_camera->cam_right;
    system->sys_camera->cam_dirty = true;
  }
  if (glfwGetKey(system->sys_window, GLFW_KEY_SPACE) == GLFW_PRESS)
  {
    system->sys_camera->cam_position += system->sys_camera->cam_speed * system->sys_camera->cam_up;
    system->sys_camera->cam_dirty = true;
  }
  if (glfwGetKey(system->sys_window, GLFW_KEY_C) == GLFW_PRESS)
  {
    system->sys_camera->cam_position -= system->sys_camera->cam_speed * system->sys_camera->cam_up;
    system->sys_camera->cam_dirty = true;
  }

  if (glfwGetMouseButton(system->sys_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
  {
    double x, y;
    glfwGetCursorPos(system->sys_window, &x, &y);
    system->sys_camera->MouseInput(vec2(x, y), false);
  }
  if (glfwGetMouseButton(system->sys_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
  {
    system->sys_camera->MouseInput(vec2(), true);
  }
}
