/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: MeshManager.cpp, includes declaration for MeshManager class (read OBJ files,
calculate vertex and face normals, create buffers)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 3-10-19
*/

#pragma once
#include <unordered_map>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

class Mesh
{
public:
  Mesh() = default;
  ~Mesh()
  {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    glDeleteBuffers(1, &FaceVBO);
    glDeleteVertexArrays(1, &FaceVAO);

    glDeleteBuffers(1, &VertexVBO);
    glDeleteVertexArrays(1, &VertexVAO);
  };

  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  GLuint NumberOfElements;
  bool fan;

  GLuint FaceVAO;
  GLuint FaceVBO;
  GLsizei FaceNumberOfElements;

  GLuint VertexVAO;
  GLuint VertexVBO;
  GLuint TBO;
  GLsizei VertexNumberOfElements;

  std::vector<glm::vec3> vertexNormals;
  std::vector<glm::vec3> faceNormals;

  glm::vec3 min;
  glm::vec3 max;
  glm::vec3 centroid;
};

enum class DrawType
{
  Line = 0,
  Point,
  Fill
};

class MeshManager
{
public:
  std::unordered_map<std::string, Mesh*> meshes;

  Mesh* GetMesh(const std::string& meshname);

  void AddMesh(const std::string& filename, bool genUV = false);

  bool ParseMeshFile(const std::string& filename, std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices);

  GLuint LoadTexture(const std::string& texture);

  bool ParseTexture(const std::string& texture, std::vector<GLubyte>& colours);

  static MeshManager* Instance()
  {
    if (!meshManager)
    {
      meshManager = new MeshManager();
    }

    return meshManager;
  }

  void CreateSquareMesh();
  void CreateSphereMesh(int numDivisions);
  void SetUpRing();

  static MeshManager* meshManager;
  ~MeshManager()
  {
    std::unordered_map<std::string, Mesh*>::iterator curr = meshes.begin();
    std::unordered_map<std::string, Mesh*>::iterator end = meshes.end();

    while (curr != end)
    {
      if (curr->second)
      {
        delete curr->second;
      }

      curr++;
    }
  }

private:
  MeshManager();
};