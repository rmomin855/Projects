#ifndef MODEL
#define MODEL

#pragma once
#include <glm/glm.hpp>
#include "MeshManager.h"

class Object;
class Model
{
public:
  Model(GLuint program, Mesh* mesh, const glm::vec3& colour = glm::vec3(1.0f));
  ~Model();

  void SetParent(Object* parent);
  void SetColour(const glm::vec3& colour);
  void SetColour(const float& r, const float& g, const float& b);
  void SetMesh(Mesh* mesh);
  void SetShaderProgram(GLuint program);

  Mesh* GetMesh();
  Object* GetParent();

  friend class GraphicsManager;
private:
  Object* m_parent;
  GLuint m_program;
  glm::vec3 m_colour;
  Mesh* m_mesh;
};


#endif // !MODEL
