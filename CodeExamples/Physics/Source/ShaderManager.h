#ifndef SHADER
#define SHADER

#pragma once
#include <unordered_map>
#include <string>
#include <gl/glew.h>

class ShaderManager
{
public:
  static ShaderManager* Instance()
  {
    if (!sm_singleton)
    {
      sm_singleton = new ShaderManager();
    }

    return sm_singleton;
  }
  static void DeleteInstance()
  {
    delete sm_singleton;
    sm_singleton = nullptr;
  }

  void LoadShaders(const char* name, const char* vertex_file_path, const char* fragment_file_path);
  GLuint GetShader(const char* name);

private:
  static ShaderManager* sm_singleton;
  ShaderManager();
  ~ShaderManager();
  std::unordered_map<std::string, GLuint> sm_shaders;
};

#endif // !SHADER
