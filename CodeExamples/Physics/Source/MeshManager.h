#ifndef MESH
#define MESH
#include <GL/glew.h>
#include <unordered_map>
#include <string>
#include <glm/glm.hpp>

class Mesh
{
public:
  Mesh() = default;
  ~Mesh()
  {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
  };

  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  GLuint NumberOfElements;

  std::vector<glm::vec3> vertices;
};

#pragma once
class MeshManager
{
public:
  static MeshManager* Instance()
  {
    if (!mm_singleton)
    {
      mm_singleton = new MeshManager();
    }

    return mm_singleton;
  }
  static void DeleteInstance()
  {
    delete mm_singleton;
    mm_singleton = nullptr;
  }

  Mesh* GetMesh(const std::string& mesh);
  void AddMesh(const std::string& filename);

private:
  static MeshManager* mm_singleton;
  MeshManager();
  ~MeshManager();

  bool ParseMeshFile(const std::string& filename, std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices);

  std::unordered_map<std::string, Mesh*> mm_meshes;
};


#endif // !MESH
