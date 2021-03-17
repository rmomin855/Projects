#include "fluid.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include <glm/gtx/transform.hpp>
#include <random>
#include <time.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

float windowWidth = 800.0f;
float windowHeight = 600.0f;

GLFWwindow* window = nullptr;

GLuint program = 0;
GLuint VAO = 0;
GLuint VBO = 0;

int MaxParticles = 3000;
Fluid* fluid = nullptr;
float pointSize = 10.0f;
float boundary = 20.0f;

float viscocity = 0.5f;
float mass = 1.0f;
float kernelradius = 2.0f * pointSize;
float restDensity = 8.31446f * 0.9f;
float dt = 1.0f / 120.0f;

int particles = 500;
static int frames = 0;

void mouse_callback()
{
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
  {
    double x;
    double y;
    glfwGetCursorPos(window, &x, &y);
    x -= (windowWidth * 0.5);
    y -= (windowHeight * 0.5);
    x *= 2.0;
    y *= 2.0;
    y = -y;

    vec2 mousePos = vec2(x, y);

    for (size_t i = 0; i < fluid->f_numberOfParticles; i++)
    {
      vec2 vecDir = fluid->f_positions[i] - mousePos;
      if (length2(vecDir) < 5000.0f)
      {
        vec2 dir = normalize(vecDir);
        fluid->f_particles[i].fp_velocity += dir * 500.0f;
      }
    }
  }
}

void Setup(int numParticles)
{
  size_t x = size_t(sqrt(numParticles));
  size_t y = x;

  srand(time(0));

  std::vector<vec2>positions(numParticles, vec2());

  vec2 top = vec2(-windowWidth + boundary, windowHeight - boundary);
  top -= 30.0f;

  for (size_t i = 0; i < numParticles; i++)
  {
    float x = (float(rand() / float(RAND_MAX)) * 200.0f) - 100.0f - 100.0f;
    float y = (float(rand() / float(RAND_MAX)) * 200.0f) - 100.0f + 60.0f;
    positions[i] = vec2(x, y);
  }

  if (fluid)
  {
    delete fluid;
    fluid = nullptr;
  }

  fluid = new Fluid(positions, viscocity, mass, kernelradius, restDensity);
}

void InitWindow(GLFWwindow*& window)
{
  int err = glfwInit();

  glfwWindowHint(GLFW_SAMPLES, 1);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(int(windowWidth), int(windowHeight),
    "PHY300 - Rahil Momin",
    nullptr,
    nullptr);

  glfwMakeContextCurrent(window);
  err = glewInit();
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  program = LoadShaders("Simple.vert",
    "Simple.frag");

  glCreateBuffers(1, &VBO);
  glNamedBufferStorage(VBO, MaxParticles * sizeof(vec2), 0, GL_DYNAMIC_STORAGE_BIT);

  glCreateVertexArrays(1, &VAO);
  glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(glm::vec2));
  glEnableVertexArrayAttrib(VAO, 0);
  glVertexArrayAttribFormat(VAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(VAO, 0, 0);


  const char* version = "#version 450";
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(version);
  ImGui::StyleColorsDark();
}

void Draw()
{
  glUseProgram(program);
  glBindVertexArray(VAO);

  glPointSize(pointSize);

  glNamedBufferSubData(VBO, 0, sizeof(vec2) * fluid->f_numberOfParticles, fluid->f_positions.data());

  GLint vTransformLoc = glGetUniformLocation(program, "projTransform");

  if (vTransformLoc >= 0)
  {
    mat4 proj = ortho(-windowWidth, windowWidth, -windowHeight, windowHeight, 5.0f, -5.0f);
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &proj[0][0]);
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
  glDrawArrays(GL_POINTS, 0, fluid->f_numberOfParticles);
}

void CheckBoundary()
{
  for (size_t i = 0; i < fluid->f_numberOfParticles; i++)
  {
    vec2& point = fluid->f_positions[i];
    FluidParticle& particle = fluid->f_particles[i];

    if (point.x < -(windowWidth - boundary))
    {
      point.x = -(windowWidth - boundary);
      particle.fp_velocity.x = -particle.fp_velocity.x * 0.1f;
    }
    else if (point.x > (windowWidth - boundary))
    {
      point.x = (windowWidth - boundary);
      particle.fp_velocity.x = -particle.fp_velocity.x * 0.1f;
    }

    if (point.y < -(windowHeight - boundary))
    {
      point.y = -(windowHeight - boundary);
      particle.fp_velocity.y = -particle.fp_velocity.y * 0.1f;
    }
    else if (point.y > (windowHeight - boundary))
    {
      point.y = (windowHeight - boundary);
      particle.fp_velocity.y = -particle.fp_velocity.y * 0.1f;
    }
  }
}

void IMGUI()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("Variables");

  float old = fluid->f_size;
  ImGui::InputFloat("Kernal radius", &fluid->f_size, 0.1f);

  if (fluid->f_size != old)
  {
    if (fluid->f_size < pointSize)
    {
      fluid->f_size = pointSize;
    }

    fluid->f_sizeSQ = fluid->f_size * fluid->f_size;

    float PI = float(M_PI);
    fluid->mullerPoly = 315.f / (65.f * PI * pow(fluid->f_size, 9.f));
    fluid->spikyGrad = -45.f / (PI * pow(fluid->f_size, 6.f));
    fluid->vicsGrad = 45.f / (PI * pow(fluid->f_size, 6.f));
  }
  kernelradius = fluid->f_size;

  ImGui::InputFloat("Mass of particle", &fluid->f_mass, 0.01f);

  if (fluid->f_mass < 0.1f)
  {
    fluid->f_mass = 0.1f;
  }
  mass = fluid->f_mass;

  ImGui::InputFloat("Viscosity", &fluid->f_visconsity, 0.01f);

  if (fluid->f_visconsity < 0.0f)
  {
    fluid->f_visconsity = 0.0f;
  }

  viscocity = fluid->f_visconsity;

  ImGui::InputFloat("Rest Density", &fluid->f_restDensity, 0.1f);

  if (fluid->f_restDensity < 1.0f)
  {
    fluid->f_restDensity = 1.0f;
  }

  restDensity = fluid->f_restDensity;

  ImGui::InputInt("Number of particles (On Reset)", &particles);

  if (particles > MaxParticles)
  {
    particles = MaxParticles;
  }

  if (ImGui::Button("Reset"))
  {
    Setup(particles);
  }

  ImGui::InputFloat("Time Step", &dt, 0.01f);

  ImGui::End();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SnapShot()
{
  static bool write = false;
  if (frames == 20)
  {
    ofstream outfile("velocitiesIni.txt");

    outfile << frames * dt << std::endl;

    for (size_t i = 0; i < fluid->f_numberOfParticles; i++)
    {
      outfile << length(fluid->f_particles[i].fp_velocity) << std::endl;
    }
  }

  if ((frames * dt) >= 10.0f && !write)
  {
    write = true;
    ofstream outfile("velocities.txt");

    outfile << frames * dt << std::endl;

    for (size_t i = 0; i < fluid->f_numberOfParticles; i++)
    {
      outfile << length(fluid->f_particles[i].fp_velocity) << std::endl;
    }
  }

  frames++;
}

int main()
{
  InitWindow(window);
  Setup(particles);

  do
  {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 0.0);

    fluid->Update(dt);
    Draw();
    CheckBoundary();
    IMGUI();
    mouse_callback();

    //SnapShot();

    glfwSwapBuffers(window);
    glfwPollEvents();
  } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);


  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}