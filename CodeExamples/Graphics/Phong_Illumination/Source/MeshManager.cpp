/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: MeshManager.cpp, includes implementations for MeshManager class (read OBJ files,
calculate vertex and face normals, create buffers)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 3-10-19
*/

#include "MeshManager.h"
#include <fstream>
#include <vector>
#include <sstream>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <iostream>

extern int TextureSize;
extern glm::vec3 Spheredistance;

std::vector<glm::vec2> UVCylindricalPos;
std::vector<glm::vec2> UVCylindricalNorm;

std::vector<glm::vec2> UVSphericalPos;
std::vector<glm::vec2> UVSphericalNorm;

std::vector<glm::vec2> UVPlanarPos;
std::vector<glm::vec2> UVPlanarNorm;
int currentMapping = 0;
int currentEntity = 0;

void APIENTRY openGLDebug(GLenum source, GLenum type, GLuint id,
  GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  switch (severity)
  {
  case GL_DEBUG_SEVERITY_HIGH:
  case GL_DEBUG_SEVERITY_MEDIUM:
    //case GL_DEBUG_SEVERITY_LOW:
    //std::cout << message << std::endl;
    break;
  default:
    break;
  }
}

MeshManager* MeshManager::meshManager = nullptr;
extern std::vector<glm::vec3> Ringvertices;

glm::vec2 CalculateCubeUV(glm::vec3 entity)
{
  glm::vec2 temp(0.0f);

  if (std::abs(entity.x) >= std::abs(entity.y) && std::abs(entity.x) >= std::abs(entity.z))
  {
    temp.x = entity.y / std::abs(entity.x);
    temp.y = entity.z / std::abs(entity.x);
  }
  else if (std::abs(entity.y) >= std::abs(entity.x) && std::abs(entity.y) >= std::abs(entity.z))
  {
    temp.x = entity.x / std::abs(entity.y);
    temp.y = entity.z / std::abs(entity.y);
  }
  else if (std::abs(entity.z) >= std::abs(entity.x) && std::abs(entity.z) >= std::abs(entity.y))
  {
    temp.x = entity.x / std::abs(entity.z);
    temp.y = entity.y / std::abs(entity.z);
  }

  temp += 1.0f;
  temp *= 0.5f;

  return temp;
}

void CalculateUV(std::vector<glm::vec3>& vertices, glm::vec3 min, glm::vec3 max, std::vector<glm::vec3>& normals)
{
  UVCylindricalPos.clear();
  UVCylindricalNorm.clear();

  UVSphericalPos.clear();
  UVSphericalNorm.clear();

  UVPlanarPos.clear();
  UVPlanarNorm.clear();

  size_t size = vertices.size();
  for (size_t i = 0; i < size; i++)
  {
    glm::vec2 uv;
    glm::vec3 temp = vertices[i];

    //cylindrical position
    float angle = std::atan2f(temp.y, temp.x);
    angle *= 2.0f;
    angle += glm::radians(180.0f);

    float z = (temp.z - min.z) / (max.z - min.z);

    uv.x = angle / glm::radians(360.0f);
    uv.y = z;

    UVCylindricalPos.push_back(uv);
    ////////////////////////////////////////////////////////////////////
    //spherical position
    float phi = std::acosf(z / glm::length(temp));
    phi /= glm::radians(180.0f);
    uv.y = 1.0f - phi;

    UVSphericalPos.push_back(uv);
    ////////////////////////////////////////////////////////////////////
    //planar position
    uv = CalculateCubeUV(vertices[i]);
    UVPlanarPos.push_back(uv);
  }


  for (size_t i = 0; i < size; i++)
  {
    glm::vec2 uv;
    glm::vec3 temp = normals[i];

    //cylindrical normal
    float angle = std::atan2f(temp.y, temp.x);
    angle *= 2.0f;
    angle += glm::radians(180.0f);

    float z = (temp.z - min.z) / (max.z - min.z);

    uv.x = angle / glm::radians(360.0f);
    uv.y = z;

    UVCylindricalNorm.push_back(uv);
    ////////////////////////////////////////////////////////////////////
    //spherical normal
    float phi = std::acosf(z / glm::length(temp));
    phi /= glm::radians(180.0f);
    uv.y = 1.0f - phi;

    UVSphericalNorm.push_back(uv);
    ////////////////////////////////////////////////////////////////////
    //planar normal
    uv = CalculateCubeUV(normals[i]);
    UVPlanarNorm.push_back(uv);
  }
}

void SetUpModelBuffers(std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices, Mesh* currentMesh)
{
  ////NORMAL BUFFERS
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  currentMesh->NumberOfElements = indices.size();

  size_t bufferSize = (vertices.size() * sizeof(glm::vec3)) * 2;

  glCreateBuffers(1, &currentMesh->VBO); // glGenBuffers + glBindBuffer

  glNamedBufferStorage(currentMesh->VBO, // same as glBufferData
    bufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);

  glNamedBufferSubData(currentMesh->VBO, 0,  // same as glBufferSubData
    sizeof(glm::vec3) * vertices.size(), vertices.data());

  glNamedBufferSubData(currentMesh->VBO, sizeof(glm::vec3) * vertices.size(),  // same as glBufferSubData
    sizeof(glm::vec3) * currentMesh->vertexNormals.size(), currentMesh->vertexNormals.data());

  glCreateBuffers(1, &currentMesh->TBO);

  glNamedBufferStorage(currentMesh->TBO, // same as glBufferData
    sizeof(glm::vec2) * UVCylindricalPos.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);

  glNamedBufferSubData(currentMesh->TBO, 0,  // same as glBufferSubData
    sizeof(glm::vec2) * UVCylindricalPos.size(), UVCylindricalPos.data());

  glCreateVertexArrays(1, &currentMesh->VAO); // glGenVertexArrays + glBindVertexArray

  glVertexArrayVertexBuffer(currentMesh->VAO, 0, currentMesh->VBO, 0, sizeof(glm::vec3));
  glVertexArrayVertexBuffer(currentMesh->VAO, 1, currentMesh->VBO, sizeof(glm::vec3) * vertices.size(), sizeof(glm::vec3));
  glVertexArrayVertexBuffer(currentMesh->VAO, 2, currentMesh->TBO, 0, sizeof(glm::vec2));

  glEnableVertexArrayAttrib(currentMesh->VAO, 0); // enable index 0: pos attribute
  glEnableVertexArrayAttrib(currentMesh->VAO, 1); // enable index 0: pos attribute
  glEnableVertexArrayAttrib(currentMesh->VAO, 2); // enable index 0: pos attribute

  // data format for vertex position attributes
  glVertexArrayAttribFormat(currentMesh->VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribFormat(currentMesh->VAO, 1, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribFormat(currentMesh->VAO, 2, 2, GL_FLOAT, GL_FALSE, 0);

  // associate VAO binding 0 to vertex attribute index 0
  glVertexArrayAttribBinding(currentMesh->VAO, 0, 0);
  glVertexArrayAttribBinding(currentMesh->VAO, 1, 1);
  glVertexArrayAttribBinding(currentMesh->VAO, 2, 2);

  // buffer for vertex indices
  glCreateBuffers(1, &currentMesh->EBO);
  glNamedBufferData(currentMesh->EBO, sizeof(GLuint) * currentMesh->NumberOfElements,
    reinterpret_cast<GLvoid*>(indices.data()), GL_STATIC_DRAW);
  glVertexArrayElementBuffer(currentMesh->VAO, currentMesh->EBO);
}

void SetUpVertexNormals(std::vector<glm::vec3>& vertices, Mesh* currentMesh)
{
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////VertexNormals
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
  std::vector<glm::vec3> Vertexvertices;
  size_t vertexSize = vertices.size();
  for (size_t i = 0; i < vertexSize; ++i)
  {
    Vertexvertices.push_back(vertices[i]);
    Vertexvertices.push_back(vertices[i] + 0.2f * currentMesh->vertexNormals[i]);
  }

  glCreateBuffers(1, &currentMesh->VertexVBO); // glGenBuffers + glBindBuffer
  glNamedBufferStorage(currentMesh->VertexVBO, // same as glBufferData
    sizeof(glm::vec3) * Vertexvertices.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);

  glNamedBufferSubData(currentMesh->VertexVBO, 0,  // same as glBufferSubData
    sizeof(glm::vec3) * Vertexvertices.size(), Vertexvertices.data());

  glCreateVertexArrays(1, &currentMesh->VertexVAO); // glGenVertexArrays + glBindVertexArray
  glVertexArrayVertexBuffer(currentMesh->VertexVAO, 0, currentMesh->VertexVBO, 0, sizeof(glm::vec3));
  glEnableVertexArrayAttrib(currentMesh->VertexVAO, 0); // enable index 0: pos attribute
  glVertexArrayAttribFormat(currentMesh->VertexVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(currentMesh->VertexVAO, 0, 0);

  currentMesh->VertexNumberOfElements = (GLsizei)Vertexvertices.size();
}

void SetUpFaceNormals(std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices, Mesh* currentMesh)
{
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
////FaceNormals
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
  std::vector<glm::vec3> Facevertices;
  size_t indexSize = indices.size();
  for (size_t i = 0; i < indexSize; i += 3)
  {
    glm::vec3 vec0 = vertices[indices[i]];
    glm::vec3 vec1 = vertices[indices[i + 1]];
    glm::vec3 vec2 = vertices[indices[i + 2]];

    glm::vec3 centerOfTri = vec0 + vec1 + vec2;
    centerOfTri /= 3.0f;

    glm::vec3 normal = currentMesh->faceNormals[i / 3];

    Facevertices.push_back(centerOfTri);
    Facevertices.push_back(centerOfTri + 0.2f * normal);
  }

  glCreateBuffers(1, &currentMesh->FaceVBO); // glGenBuffers + glBindBuffer
  glNamedBufferStorage(currentMesh->FaceVBO, // same as glBufferData
    sizeof(glm::vec3) * Facevertices.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);

  glNamedBufferSubData(currentMesh->FaceVBO, 0,  // same as glBufferSubData
    sizeof(glm::vec3) * Facevertices.size(), Facevertices.data());

  glCreateVertexArrays(1, &currentMesh->FaceVAO); // glGenVertexArrays + glBindVertexArray
  glVertexArrayVertexBuffer(currentMesh->FaceVAO, 0, currentMesh->FaceVBO, 0, sizeof(glm::vec3));
  glEnableVertexArrayAttrib(currentMesh->FaceVAO, 0); // enable index 0: pos attribute
  glVertexArrayAttribFormat(currentMesh->FaceVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(currentMesh->FaceVAO, 0, 0);

  currentMesh->FaceNumberOfElements = (GLsizei)Facevertices.size();
}

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

MeshManager::MeshManager()
{
  CreateSphereMesh(30);
}

Mesh* MeshManager::GetMesh(const std::string& meshname)
{
  return meshes[meshname.data()];
}

void MeshManager::AddMesh(const std::string& filename, bool genUV)
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
    currentMesh->fan = true;
    currentMesh->NumberOfElements = indices.size();

    size_t vertexSize = vertices.size();
    glm::vec3 center = {};
    glm::vec3 min = { INT_MAX, INT_MAX , INT_MAX };
    glm::vec3 max = { INT_MIN, INT_MIN, INT_MIN };

    for (size_t i = 0; i < vertexSize; i++)
    {
      center += vertices[i];
    }

    center /= vertexSize;

    for (size_t i = 0; i < vertexSize; i++)
    {
      vertices[i] -= center;

      min.x = glm::min(min.x, vertices[i].x);
      min.y = glm::min(min.y, vertices[i].y);
      min.z = glm::min(min.z, vertices[i].z);

      max.x = glm::max(max.x, vertices[i].x);
      max.y = glm::max(max.y, vertices[i].y);
      max.z = glm::max(max.z, vertices[i].z);
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

      vertices[i] /= temp;
      vertices[i] *= 2.0f;
      vertices[i] -= 1.0f;

      vertices[i] *= scaleRatio;
    }

    currentMesh->vertexNormals.resize(vertices.size());

    size_t indexSize = indices.size();
    for (size_t i = 0; i < indexSize; i += 3)
    {
      glm::vec3 vec1 = vertices[indices[i + 1]] - vertices[indices[i]];
      glm::vec3 vec2 = vertices[indices[i + 2]] - vertices[indices[i]];

      glm::vec3 cross = glm::cross(vec1, vec2);
      cross = glm::normalize(cross);

      currentMesh->faceNormals.push_back(cross);

      currentMesh->vertexNormals[indices[i]] += cross;
      currentMesh->vertexNormals[indices[i + 1]] += cross;
      currentMesh->vertexNormals[indices[i + 2]] += cross;
    }

    size_t VertexNormalsSize = currentMesh->vertexNormals.size();
    for (size_t i = 0; i < VertexNormalsSize; i++)
    {
      currentMesh->vertexNormals[i] = glm::normalize(currentMesh->vertexNormals[i]);
    }

    min = { INT_MAX, INT_MAX , INT_MAX };
    max = { INT_MIN, INT_MIN, INT_MIN };

    for (size_t i = 0; i < vertexSize; i++)
    {
      min.x = glm::min(min.x, vertices[i].x);
      min.y = glm::min(min.y, vertices[i].y);
      min.z = glm::min(min.z, vertices[i].z);

      max.x = glm::max(max.x, vertices[i].x);
      max.y = glm::max(max.y, vertices[i].y);
      max.z = glm::max(max.z, vertices[i].z);
    }

    currentMesh->max = max;
    currentMesh->min = min;

    if (genUV)
    {
      CalculateUV(vertices, min, max, currentMesh->vertexNormals);
    }
    else
    {
      UVCylindricalPos.resize(vertices.size());
    }

    SetUpModelBuffers(vertices, indices, currentMesh);

    SetUpFaceNormals(vertices, indices, currentMesh);

    SetUpVertexNormals(vertices, currentMesh);

    meshes[filename] = currentMesh;
  }
}

bool MeshManager::ParseMeshFile(const std::string& filename, std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices)
{
  std::string directory = std::string("../Common/models/");
  directory += filename;
  directory += ".obj";

  std::filebuf file;

  //open file
  if (file.open(directory.data(), std::ios::in))
  {
    std::istream fileIn(&file);

    fileIn.seekg(0, fileIn.end);
    size_t length = fileIn.tellg();
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
      size_t curr = fileIn.tellg();
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

void MeshManager::CreateSquareMesh()
{
  Mesh* square = new Mesh();

  /////////////////////////////////////////////////////////////////////////

  std::vector<glm::vec3> geometryBuffer =
  {
  {0.0f, 0.0f, 0.0f},
  {1.0f, 0.0f, 0.0f},
  {0.0f, 1.0f, 0.0f},
  {1.0f, 1.0f, 0.0f},
  };

  std::vector<GLuint> indices = { 0,1,3,0,3,2 };

  GLuint vertex_count = geometryBuffer.size();
  square->NumberOfElements = static_cast<GLuint>(indices.size());

  glCreateBuffers(1, &square->VBO); // glGenBuffers + glBindBuffer
  glNamedBufferStorage(square->VBO, // same as glBufferData
    sizeof(glm::vec3) * vertex_count, nullptr, GL_DYNAMIC_STORAGE_BIT);

  glNamedBufferSubData(square->VBO, 0,  // same as glBufferSubData
    sizeof(glm::vec3) * vertex_count, geometryBuffer.data());

  glCreateVertexArrays(1, &square->VAO); // glGenVertexArrays + glBindVertexArray

  glVertexArrayVertexBuffer(square->VAO, 0, square->VBO, 0, sizeof(glm::vec3));

  glEnableVertexArrayAttrib(square->VAO, 0); // enable index 0: pos attribute
  // data format for vertex position attributes
  glVertexArrayAttribFormat(square->VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

  // associate VAO binding 0 to vertex attribute index 0
  glVertexArrayAttribBinding(square->VAO, 0, 0);

  // buffer for vertex indices
  glCreateBuffers(1, &square->EBO);
  glNamedBufferData(square->EBO, sizeof(GLuint) * square->NumberOfElements,
    reinterpret_cast<GLvoid*>(indices.data()), GL_STATIC_DRAW);
  glVertexArrayElementBuffer(square->VAO, square->EBO);
  ///////////////////////////////////////////////////////////////////////////

  meshes["square"] = square;
}

void MeshManager::CreateSphereMesh(int numDivisions)
{
  Mesh* currentMesh = new Mesh();

  std::vector<glm::vec3> vertices;
  std::vector<GLuint> indices;

  // vertex position
  float phiHigh;
  glm::vec3 vertex;

  float thetaStep = 2.0f * float(M_PI) / numDivisions;
  float phiStep = float(M_PI) / numDivisions;
  float theta, phi;

  for (int i = 0; i <= numDivisions; ++i)
  {
    phi = float(M_PI) / 2.0f - i * phiStep;        // starting from pi/2 to -pi/2
    phiHigh = cosf(phi);             // r * cos(u)
    vertex.z = sinf(phi);              // r * sin(u)

    // add (sectorCount+1) vertices per stack
    // the first and last vertices have same position and normal, but different tex coords
    for (int j = 0; j <= numDivisions; ++j)
    {
      theta = j * thetaStep;           // starting from 0 to 2pi

      // vertex position (x, y, z)
      vertex.x = phiHigh * cosf(theta);             // r * cos(u) * cos(v)
      vertex.y = phiHigh * sinf(theta);             // r * cos(u) * sin(v)
      vertices.push_back(vertex);
    }
  }

  GLuint k1, k2;
  for (int i = 0; i < numDivisions; ++i)
  {
    k1 = i * (numDivisions + 1);     // beginning of current stack
    k2 = k1 + numDivisions + 1;      // beginning of next stack

    for (int j = 0; j < numDivisions; ++j, ++k1, ++k2)
    {
      // 2 triangles per sector excluding first and last stacks
      // k1 => k2 => k1+1
      if (i != 0)
      {
        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);
      }

      // k1+1 => k2 => k2+1
      if (i != (numDivisions - 1))
      {
        indices.push_back(k1 + 1);
        indices.push_back(k2);
        indices.push_back(k2 + 1);
      }
    }
  }

  currentMesh->vertexNormals.resize(vertices.size());

  size_t indexSize = indices.size();
  for (size_t i = 0; i < indexSize; i += 3)
  {
    glm::vec3 vec1 = vertices[indices[i + 1]] - vertices[indices[i]];
    glm::vec3 vec2 = vertices[indices[i + 2]] - vertices[indices[i]];

    glm::vec3 cross = glm::cross(vec1, vec2);
    cross = glm::normalize(cross);

    currentMesh->faceNormals.push_back(cross);

    currentMesh->vertexNormals[indices[i]] += cross;
    currentMesh->vertexNormals[indices[i + 1]] += cross;
    currentMesh->vertexNormals[indices[i + 2]] += cross;
  }

  size_t VertexNormalsSize = currentMesh->vertexNormals.size();
  for (size_t i = 0; i < VertexNormalsSize; i++)
  {
    currentMesh->vertexNormals[i] = glm::normalize(currentMesh->vertexNormals[i]);
  }

  CalculateUV(vertices, glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), currentMesh->vertexNormals);
  SetUpModelBuffers(vertices, indices, currentMesh);
  SetUpFaceNormals(vertices, indices, currentMesh);
  SetUpVertexNormals(vertices, currentMesh);

  meshes["sphereGen"] = currentMesh;
}

void MeshManager::SetUpRing()
{
  Mesh* ring = new Mesh();

  std::vector<glm::vec3> points;

  for (float i = 0.0f; i < 360.0f; i++)
  {
    glm::vec3 temp = glm::rotateY(Spheredistance, glm::radians(i));
    points.push_back(temp);
  }

  size_t pointSize = points.size();

  for (size_t i = 0; i <= pointSize - 1; i++)
  {
    Ringvertices.push_back(points[i]);

    if (i == (pointSize - 1))
    {
      Ringvertices.push_back(points[0]);
      break;
    }
  }

  glCreateBuffers(1, &ring->VertexVBO); // glGenBuffers + glBindBuffer
  glNamedBufferStorage(ring->VertexVBO, // same as glBufferData
    sizeof(glm::vec3) * Ringvertices.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);

  glNamedBufferSubData(ring->VertexVBO, 0,  // same as glBufferSubData
    sizeof(glm::vec3) * Ringvertices.size(), Ringvertices.data());

  glCreateVertexArrays(1, &ring->VertexVAO); // glGenVertexArrays + glBindVertexArray
  glVertexArrayVertexBuffer(ring->VertexVAO, 0, ring->VertexVBO, 0, sizeof(glm::vec3));
  glEnableVertexArrayAttrib(ring->VertexVAO, 0); // enable index 0: pos attribute
  glVertexArrayAttribFormat(ring->VertexVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(ring->VertexVAO, 0, 0);

  ring->VertexNumberOfElements = (GLsizei)Ringvertices.size();

  meshes["ring"] = ring;
}

bool MeshManager::ParseTexture(const std::string& texture, std::vector<GLubyte>& colours)
{
  std::string directory = std::string("../Common/textures/");
  directory += texture;
  directory += ".ppm";

  std::fstream file;
  file.open(directory.data(), std::ios::in);

  if (!file.is_open())
  {
    return false;
  }

  std::string line;
  int curr = 0;

  bool first = true;

  while (std::getline(file, line))
  {
    if (line[0] < '0' || line[0] > '9')
    {
      continue;
    }

    if (line.size() > 3)
    {
      continue;
    }

    if (first)
    {
      first = false;
      continue;
    }

    colours.push_back(atoi(line.data()));
  }

  return true;
}

GLuint MeshManager::LoadTexture(const std::string& texture)
{
  std::vector<GLubyte> colours;
  if (!ParseTexture(texture, colours))
  {
    return -1;
  }

  GLuint texBuffer = -1;
  glCreateTextures(GL_TEXTURE_2D, 1, &texBuffer);

  glTextureStorage2D(texBuffer, 1, GL_RGB8, TextureSize, TextureSize);
  glTextureSubImage2D(texBuffer, 0, 0, 0, TextureSize, TextureSize, GL_RGB, GL_UNSIGNED_BYTE, colours.data());

  glGenerateMipmap(GL_TEXTURE_2D);

  return texBuffer;
}
