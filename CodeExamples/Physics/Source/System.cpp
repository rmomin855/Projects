#include "System.h"
#include <chrono> 
#include <ctime> 
#include <iostream>

System* System::sys_singleton = nullptr;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  GraphicsManager::Instance()->UpdateFOV(float(-yoffset));
}

System::System(unsigned int width, unsigned int height)
  : sys_isRunning(true), sys_screenWidth(width), sys_screenHeight(height), sys_window(),
  sys_sceneManager(), sys_inputManager(), sys_meshManager(), sys_paused(false), sys_step(false)
  , sys_shaderManager(), sys_graphicsManager(), sys_camera(), sys_physicsManager()
{
  start = std::chrono::system_clock::now();
}

System::~System()
{
  SceneManager::DeleteInstance();
  InputManager::DeleteInstance();
  MeshManager::DeleteInstance();
  ShaderManager::DeleteInstance();
  GraphicsManager::DeleteInstance();
  PhysicsManager::DeleteInstance();

  delete sys_camera;
}

void System::Init()
{
  if (!glfwInit())
  {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return;
  }

  glfwWindowHint(GLFW_SAMPLES, 1);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  sys_window = glfwCreateWindow(sys_screenWidth, sys_screenHeight,
    "CS550",
    nullptr,
    nullptr);
  if (sys_window == nullptr)
  {
    fprintf(stderr,
      "Failed to open GLFW window. If you have an Intel GPU, it is not 4.6 compatible.\n");
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(sys_window);

  glewExperimental = static_cast<GLboolean>(true);
  if (glewInit() != GLEW_OK)
  {
    fprintf(stderr, "Failed to initialize GLEW\n");
    glfwTerminate();
    return;
  }

  glfwSetInputMode(sys_window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetInputMode(sys_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  glfwSetScrollCallback(sys_window, scroll_callback);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);

  sys_sceneManager = SceneManager::Instance();
  sys_inputManager = InputManager::Instance();
  sys_meshManager = MeshManager::Instance();
  sys_shaderManager = ShaderManager::Instance();
  sys_graphicsManager = GraphicsManager::Instance();
  sys_physicsManager = PhysicsManager::Instance();

  sys_camera = new Camera(vec3(0.0f, 0.0f, 20.0f));
}

void System::Update(float dt)
{
  sys_inputManager->Update(this);

  if (!sys_paused || sys_step)
  {
    sys_graphicsManager->FlushDebug();
    sys_sceneManager->Update(dt);
    sys_physicsManager->Update(dt);

    sys_step = false;
  }

  sys_graphicsManager->Render(sys_camera);

  glfwSwapBuffers(sys_window);
  glfwPollEvents();

  frames++;
  if (frames == 60)
  {
    frames = 0;
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "FPS: " << 60.0f / elapsed_seconds.count() << std::endl;
    start = end;
  }
}

void System::cleanup()
{
  glfwTerminate();
}

bool System::Running()
{
  return sys_isRunning;
}

void System::Stop()
{
  sys_isRunning = false;
}

GLFWwindow* System::GetWindow()
{
  return sys_window;
}

Camera* System::GetCamera()
{
  return sys_camera;
}
