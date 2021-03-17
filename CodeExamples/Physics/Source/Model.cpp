#include "Model.h"
#include "GraphicsManager.h"

Model::Model(GLuint program, Mesh* mesh, const glm::vec3& colour)
  : m_program(program), m_mesh(mesh), m_colour(colour), m_parent()
{
  GraphicsManager::Instance()->AddToList(this);
}

Model::~Model()
{
  GraphicsManager::Instance()->RemoveFromList(this);
}

void Model::SetParent(Object* parent)
{
  m_parent = parent;
}

void Model::SetColour(const glm::vec3& colour)
{
  m_colour = colour;
}

void Model::SetColour(const float& r, const float& g, const float& b)
{
  m_colour = vec3(r, g, b);
}

void Model::SetMesh(Mesh* mesh)
{
  m_mesh = mesh;
}

void Model::SetShaderProgram(GLuint program)
{
  m_program = program;
}

Mesh* Model::GetMesh()
{
  return m_mesh;
}

Object* Model::GetParent()
{
  return m_parent;
}
