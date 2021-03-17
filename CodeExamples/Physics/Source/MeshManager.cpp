#include "MeshManager.h"
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace glm;
MeshManager* MeshManager::mm_singleton = nullptr;

std::vector<std::string> parseBuffer(char* buffer)
{
  std::vector<std::string> v;

  char* temp;

  //tokenize the buffer
  temp = strtok(buffer, ": ,/\\\"\n\tpar");

  while (temp != NULL)
  {
    std::string stringtemp = temp;
    v.push_back(stringtemp);
    temp = strtok(NULL, ": ,/\\\"\n\tpar");
  }

  return v;
}

void SetUpModelBuffers(std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices, Mesh* currentMesh)
{
  size_t bufferSize = (vertices.size() * sizeof(glm::vec3));

  glCreateBuffers(1, &currentMesh->VBO); // glGenBuffers + glBindBuffer

  glNamedBufferStorage(currentMesh->VBO, // same as glBufferData
    bufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);

  glNamedBufferSubData(currentMesh->VBO, 0,  // same as glBufferSubData
    sizeof(glm::vec3) * vertices.size(), vertices.data());

  glCreateVertexArrays(1, &currentMesh->VAO); // glGenVertexArrays + glBindVertexArray

  glVertexArrayVertexBuffer(currentMesh->VAO, 0, currentMesh->VBO, 0, sizeof(glm::vec3));

  glEnableVertexArrayAttrib(currentMesh->VAO, 0); // enable index 0: pos attribute

  // data format for vertex position attributes
  glVertexArrayAttribFormat(currentMesh->VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

  // associate VAO binding 0 to vertex attribute index 0
  glVertexArrayAttribBinding(currentMesh->VAO, 0, 0);

  // buffer for vertex indices
  glCreateBuffers(1, &currentMesh->EBO);
  glNamedBufferData(currentMesh->EBO, sizeof(GLuint) * currentMesh->NumberOfElements,
    reinterpret_cast<GLvoid*>(indices.data()), GL_STATIC_DRAW);
  glVertexArrayElementBuffer(currentMesh->VAO, currentMesh->EBO);
}

Mesh* MeshManager::GetMesh(const std::string& mesh)
{
  return mm_meshes[mesh];
}

void MeshManager::AddMesh(const std::string& filename)
{
  if (!filename.size() || GetMesh(filename))
  {
    return;
  }

  std::vector<glm::vec3> vertices;
  std::vector<GLuint> indices;

  if (ParseMeshFile(filename, vertices, indices))
  {
    Mesh* currentMesh = new Mesh();
    currentMesh->NumberOfElements = indices.size();

    size_t vertexSize = vertices.size();
    glm::vec3 center = {};
    glm::vec3 min = { INT_MAX, INT_MAX , INT_MAX };
    glm::vec3 max = { INT_MIN, INT_MIN, INT_MIN };

    for (size_t i = 0; i < vertexSize; i++)
    {
      center += vertices[i];
    }

    center /= float(vertexSize);

    for (size_t i = 0; i < vertexSize; i++)
    {
      vertices[i] -= center;

      min = glm::min(min, vertices[i]);
      max = glm::max(max, vertices[i]);
    }

    float xlen = max.x - min.x;
    float ylen = max.y - min.y;
    float zlen = max.z - min.z;

    float greatest = std::max(std::max(xlen, ylen), zlen);

    glm::vec3 scaleRatio = glm::vec3(1.0f, 1.0f, 1.0f);

    if (greatest == xlen)
    {
      scaleRatio.y = ylen / greatest;
      scaleRatio.z = zlen / greatest;
    }
    if (greatest == ylen)
    {
      scaleRatio.x = xlen / greatest;
      scaleRatio.z = zlen / greatest;
    }
    if (greatest == zlen)
    {
      scaleRatio.y = ylen / greatest;
      scaleRatio.x = xlen / greatest;
    }

    for (size_t i = 0; i < vertexSize; i++)
    {
      vertices[i] -= min;

      glm::vec3 temp = max - min;
      if (!temp.z)
      {
        temp.z = 1.0f;
      }
      if (!temp.y)
      {
        temp.y = 1.0f;
      }
      if (!temp.x)
      {
        temp.x = 1.0f;
      }

      vertices[i] /= temp;
      vertices[i] *= 2.0f;
      vertices[i] -= 1.0f;

      vertices[i] *= scaleRatio;
    }

    SetUpModelBuffers(vertices, indices, currentMesh);

    currentMesh->vertices = vertices;
    mm_meshes[filename] = currentMesh;
  }
}

MeshManager::MeshManager()
{
  AddMesh("cube2");
  AddMesh("sphere");
  AddMesh("4Sphere");
  AddMesh("bunny");
}

MeshManager::~MeshManager()
{
  std::unordered_map<std::string, Mesh*>::iterator curr = mm_meshes.begin();
  std::unordered_map<std::string, Mesh*>::iterator end = mm_meshes.end();

  while (curr != end)
  {
    if (curr->second)
    {
      delete curr->second;
    }

    curr++;
  }
}

bool MeshManager::ParseMeshFile(const std::string& filename, std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices)
{
  std::string directory = std::string("../Models/");
  directory += filename;
  directory += ".obj";

  std::filebuf file;

  //open file
  if (file.open(directory.data(), std::ios::in))
  {
    std::istream fileIn(&file);

    fileIn.seekg(0, fileIn.end);
    size_t length = size_t(fileIn.tellg());
    fileIn.seekg(0, fileIn.beg);

    //file is open
    while (file.is_open())
    {
      char temp[300] = { 0 };
      //read line
      fileIn.getline(temp, 300);

      //get tokens
      std::vector<std::string> tokens = parseBuffer(temp);

      if (tokens.size())
      {
        //vertex
        if (tokens[0][0] == 'v')
        {
          glm::vec3 vertex;
          vertex.x = (float)std::atof(tokens[1].data());
          vertex.y = (float)std::atof(tokens[2].data());
          vertex.z = (float)std::atof(tokens[3].data());

          vertices.push_back(vertex);
        }
        //fan vertex
        else if (tokens[0][0] == 'f')
        {
          indices.push_back((GLuint)std::atoi(tokens[1].data()) - 1);
          indices.push_back((GLuint)std::atoi(tokens[2].data()) - 1);
          indices.push_back((GLuint)std::atoi(tokens[3].data()) - 1);
        }
      }

      //if invalid line, close file
      size_t curr = size_t(fileIn.tellg());
      if (curr >= length)
      {
        file.close();
        break;
      }

    }

    return true;
  }
  else
  {
    return false;
  }
}
