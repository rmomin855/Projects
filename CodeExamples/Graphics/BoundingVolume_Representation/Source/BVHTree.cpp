/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: BVHTree.cpp, implementation for BVHTree class (maintains BVH and calculates bounding volumes)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS350
Author: Rahil Momin, r.momin, 180001717
Creation date: 7-3-20
*/

#include "BVHTree.h"
#include "Node.h"
#include "MeshManager.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

#include <glm/gtx/norm.hpp>
#include "SimpleScene_Quad.h"

extern Camera camera;
extern Scene* scene;
extern glm::mat4 pers;
extern Scene* scene;

BVHTree* BVHTree::tree = nullptr;

mat3 CovarianceMatrix(std::vector<vec3>& pt, int numPts, vec3 centroid)
{
  mat3 cov;
  float oon = 1.0f / (float)numPts;
  float e00, e11, e22, e01, e02, e12;
  e00 = e11 = e22 = e01 = e02 = e12 = 0.0f;

  // Compute covariance elements
  for (int i = 0; i < numPts; i++)
  {
    // Translate points so center of mass is at origin
    vec3 p = pt[i] - centroid;
    // Compute covariance of translated points
    e00 += p.x * p.x;
    e11 += p.y * p.y;
    e22 += p.z * p.z;
    e01 += p.x * p.y;
    e02 += p.x * p.z;
    e12 += p.y * p.z;
  }
  // Fill in the covariance matrix elements
  cov[0][0] = e00 * oon;
  cov[1][1] = e11 * oon;
  cov[2][2] = e22 * oon;
  cov[0][1] = cov[1][0] = e01 * oon;
  cov[0][2] = cov[2][0] = e02 * oon;
  cov[1][2] = cov[2][1] = e12 * oon;

  return cov;
}

void SymSchur2(mat3& a, int p, int q, float& c, float& s)
{
  if (std::abs(a[p][q]) > 0.0001f)
  {
    float r = (a[q][q] - a[p][p]) / (2.0f * a[p][q]);
    float t;
    if (r >= 0.0f)
      t = 1.0f / (r + sqrtf(1.0f + r * r));
    else
      t = -1.0f / (-r + sqrtf(1.0f + r * r));
    c = 1.0f / sqrtf(1.0f + t * t);
    s = t * c;
  }
  else
  {
    c = 1.0f;
    s = 0.0f;
  }
}

void Jacobi(mat3& a, mat3& v)
{
  int i, j, n, p, q;
  float prevoff, c, s;
  mat3 J;
  // Initialize v to identify matrix
  for (i = 0; i < 3; i++) {
    v[i][0] = v[i][1] = v[i][2] = 0.0f;
    v[i][i] = 1.0f;
  }
  // Repeat for some maximum number of iterations
  const int MAX_ITERATIONS = 100;
  for (n = 0; n < MAX_ITERATIONS; n++) {
    // Find largest off-diagonal absolute element a[p][q]
    p = 0; q = 1;
    for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
        if (i == j) continue;
        if (abs(a[i][j]) > abs(a[p][q])) {
          p = i;
          q = j;
        }
      }
    }
    // Compute the Jacobi rotation matrix J(p, q, theta)
    // (This code can be optimized for the three different cases of rotation)
    SymSchur2(a, p, q, c, s);
    for (i = 0; i < 3; i++) {
      J[i][0] = J[i][1] = J[i][2] = 0.0f;
      J[i][i] = 1.0f;
    }
    J[p][p] = c; J[p][q] = s;
    J[q][p] = -s; J[q][q] = c;
    // Cumulate rotations into what will contain the eigenvectors
    v = v * J;
    // Make ’a’ more diagonal, until just eigenvalues remain on diagonal
    a = (glm::transpose(J) * a) * J;
    // Compute "norm" of off-diagonal elements
    float off = 0.0f;
    for (i = 0; i < 3; i++)
    {
      for (j = 0; j < 3; j++)
      {
        if (i == j) continue;
        off += a[i][j] * a[i][j];
      }
    }
    /* off = sqrt(off); not needed for norm comparison */
    // Stop when norm no longer decreasing
    if (n > 2 && off >= prevoff)
      return;
    prevoff = off;
  }
}

vec3 GetColour(Colours colour)
{
  switch (colour)
  {
  case Colours::Red:
    return vec3(1.0f, 0.0f, 0.0f);
  case Colours::Blue:
    return vec3(0.0f, 0.0f, 1.0f);
  case Colours::White:
    return vec3(1.0f, 1.0f, 1.0f);
  case Colours::Green:
    return vec3(0.0f, 1.0f, 0.0f);
  case Colours::Purple:
    return vec3(0.4f, 0.0f, 0.8f);
  case Colours::Orange:
    return vec3(1.0f, 0.55f, 0.0f);
  case Colours::Yellow:
    return vec3(1.0f, 1.0f, 0.0f);
  }

  return vec3();
}

BVHTree::~BVHTree()
{
  ClearTree();
}

void BVHTree::DrawBVH()
{
  volumes.clear();

  if (root)
  {
    root->DrawDebug(Colours::Red);
  }

  DrawDebug();
}

void BVHTree::UpdateRoot(Node* node)
{
  root = node;
}

void BVHTree::BuildTreeBottomUp(std::vector<Mesh*>& meshes)
{
  std::vector<Mesh*> current = meshes;
  nodes.clear();
  size_t size = meshes.size();

  for (size_t i = 0; i < size; i++)
  {
    nodes.push_back(meshes[i]->node);
  }

  //get pair from with first element
  while (current.size())
  {
    Node* pair1 = current[0]->node;
    Node* pair2 = pair1;
    const vec3& position1 = current[0]->centroid;

    float closestLen = float(UINT32_MAX);

    for (size_t i = 1; i < current.size(); i++)
    {
      const vec3& position2 = current[i]->centroid;

      float dist = glm::distance(position1, position2);

      if (dist < closestLen)
      {
        closestLen = dist;
        pair2 = current[i]->node;
      }
    }

    if (pair1 != pair2)
    {
      Node* newNode = new Node(vec3(), vec3());
      newNode->n_left = pair1;
      pair1->n_parent = newNode;
      newNode->n_right = pair2;
      pair2->n_parent = newNode;

      newNode->UpdateNode();

      tree->AddPairs(newNode);

      current.erase(current.begin());

      auto curr = current.begin();
      auto end = current.end();

      while (curr != end)
      {
        if ((*curr)->node == pair2)
        {
          break;
        }
        ++curr;
      }
      current.erase(curr);
    }
    else
    {
      Node* newNode = new Node(vec3(), vec3());
      newNode->n_left = pair1;
      pair1->n_parent = newNode;
      newNode->n_right = nullptr;

      newNode->UpdateNode();

      tree->AddPairs(newNode);
      current.erase(current.begin());
    }
  }
}

void BVHTree::BuildTreeTopDown(std::vector<glm::vec3>& vertexPool, std::vector<glm::vec3>& normalPool,
  int level, Axis axis, Node* parent, bool leftChild)
{
  if (axis == Axis::numAxis)
  {
    axis = Axis::X;
  }

  vec3 max = vec3(float(INTMAX_MIN));
  vec3 min = vec3(float(INTMAX_MAX));

  size_t size = vertexPool.size();

  for (size_t i = 0; i < size; i++)
  {
    max = glm::max(max, vertexPool[i]);
    min = glm::min(min, vertexPool[i]);
  }

  Node* node = new Node(min, max);
  if (!parent)
  {
    root = node;
  }
  else
  {
    node->n_parent = parent;

    if (leftChild)
    {
      parent->n_left = node;
    }
    else
    {
      parent->n_right = node;
    }
  }

  if ((cutoff == CutOff::Height && level == 7) || (cutoff == CutOff::Vertices && size <= 500))
  {
    Mesh* mesh = new Mesh();
    mesh->vertex = vertexPool;
    mesh->vertexNormals = normalPool;
    mesh->node = node;
    node->n_object = mesh;
    node->n_leaf = true;

    for (size_t i = 0; i < size; i++)
    {
      mesh->centroid += vertexPool[i];
    }

    mesh->centroid /= size;

    nodes.push_back(node);
    return;
  }

  vec3 center = (max + min) * 0.5f;
  std::vector<vec3>leftVertices;
  std::vector<vec3>rightVertices;
  std::vector<vec3>leftNormals;
  std::vector<vec3>rightNormals;

  for (size_t i = 0; i < size; i++)
  {
    if (axis == Axis::X)
    {
      if (vertexPool[i].x > center.x)
      {
        rightVertices.push_back(vertexPool[i]);
        rightNormals.push_back(normalPool[i]);
      }
      else
      {
        leftVertices.push_back(vertexPool[i]);
        leftNormals.push_back(normalPool[i]);
      }
    }
    else if (axis == Axis::Y)
    {
      if (vertexPool[i].y > center.y)
      {
        rightVertices.push_back(vertexPool[i]);
        rightNormals.push_back(normalPool[i]);
      }
      else
      {
        leftVertices.push_back(vertexPool[i]);
        leftNormals.push_back(normalPool[i]);
      }
    }
    else
    {
      if (vertexPool[i].z > center.z)
      {
        rightVertices.push_back(vertexPool[i]);
        rightNormals.push_back(normalPool[i]);
      }
      else
      {
        leftVertices.push_back(vertexPool[i]);
        leftNormals.push_back(normalPool[i]);
      }
    }
  }

  if (leftVertices.size())
  {
    BuildTreeTopDown(leftVertices, leftNormals, level + 1, Axis(int(axis) + 1), node, true);
  }
  if (rightVertices.size())
  {
    BuildTreeTopDown(rightVertices, rightNormals, level + 1, Axis(int(axis) + 1), node, false);
  }

  node->UpdateNode();
}

void BVHTree::AddDebugBox(Colours colour, const glm::vec3& pos, const glm::vec3& scale, const glm::mat4& rotation)
{
  glm::mat4 matrix(1.0f);
  matrix = glm::translate(matrix, pos);
  matrix = matrix * rotation;
  matrix = glm::scale(matrix, scale);

  volumes[colour].push_back(matrix);
}

void BVHTree::AddDebugSphere(Colours colour, glm::vec3 pos, float radius)
{
  glm::mat4 matrix(1.0f);
  matrix = glm::translate(matrix, pos);
  matrix = glm::scale(matrix, glm::vec3(radius));

  volumes[colour].push_back(matrix);
}

void BVHTree::AddToTree(Node* node)
{
  if (!root)
  {
    root = node;
    return;
  }

  root->AddToNode(node);
}

void BVHTree::AddPairs(Node* node)
{
  if (!root)
  {
    root = node;
    return;
  }

  root->AddCompactPair(node);
}

void BVHTree::RemoveFromTree(Node* node)
{
  if (!node || !root)
  {
    return;
  }

  root->RemoveFromNode(node);
}

void BVHTree::CentroidMethod()
{
  for (size_t i = 0; i < nodes.size(); i++)
  {
    nodes[i]->n_center = nodes[i]->n_object->centroid;

    auto& vertices = nodes[i]->n_object->vertex;
    size_t size = vertices.size();
    nodes[i]->n_radius = 0.0f;

    for (size_t j = 0; j < size; j++)
    {
      float dist = distance(nodes[i]->n_center, vertices[j]);
      nodes[i]->n_radius = dist > nodes[i]->n_radius ? dist : nodes[i]->n_radius;
    }

    nodes[i]->UpdateNode();
  }
}

void BVHTree::RittersMethod()
{
  for (size_t i = 0; i < nodes.size(); i++)
  {
    auto& vertices = nodes[i]->n_object->vertex;
    size_t size = vertices.size();

    int index = int((float(rand()) / float(RAND_MAX)) * size);
    vec3 pointInitial = vertices[index];

    vec3 pointFurthestFrom = vec3();
    float distFrom = 0.0f;

    //find furthest point
    for (size_t j = 1; j < size; j++)
    {
      if (j == index)
      {
        continue;
      }

      float currDist = distance(pointInitial, vertices[j]);
      if (currDist > distFrom)
      {
        distFrom = currDist;
        pointFurthestFrom = vertices[j];
      }
    }

    vec3 pointFurther = vec3();
    distFrom = 0.0f;
    //find furthest point from previous point
    for (size_t j = 0; j < size; j++)
    {
      float currDist = distance(pointFurthestFrom, vertices[j]);
      if (currDist > distFrom)
      {
        distFrom = currDist;
        pointFurther = vertices[j];
      }
    }

    nodes[i]->n_center = (pointFurthestFrom + pointFurther) * 0.5f;
    nodes[i]->n_radius = length(pointFurthestFrom - pointFurther) * 0.5f;

    //if a point is outside the sphere then extend sphere
    for (size_t j = 0; j < size; j++)
    {
      float currDist = distance(nodes[i]->n_center, vertices[j]);

      if (currDist > nodes[i]->n_radius)
      {
        nodes[i]->n_radius = currDist;
      }
    }

    nodes[i]->UpdateNode();
  }
}

void BVHTree::ModLarssonMethod()
{
  for (size_t i = 0; i < nodes.size(); i++)
  {
    auto& vertices = nodes[i]->n_object->vertex;
    auto& normals = nodes[i]->n_object->vertexNormals;
    size_t size = vertices.size();

    std::vector<vec3> points;
    size_t extremeCount = size_t(float(rand()) / float(RAND_MAX) * float(vertices.size()) * 0.3f);
    extremeCount = extremeCount < 4 ? 4 : extremeCount;
    size_t count = 0;

    while (count < extremeCount)
    {
      size_t index = size_t(float(rand()) / float(RAND_MAX) * float(vertices.size()));
      index = index == vertices.size() ? index - 1 : index;

      points.push_back(vertices[index]);
      count += 2;
    }

    size_t subsetSize = points.size();
    vec3 max(float(INTMAX_MIN));
    vec3 min(float(INTMAX_MAX));
    for (size_t j = 0; j < subsetSize; j++)
    {
      nodes[i]->n_center += points[j];
      min = glm::min(min, points[j]);
      max = glm::max(max, points[j]);
    }
    nodes[i]->n_center /= subsetSize;
    nodes[i]->n_radius = length2((max - min) * 0.5f);

    for (size_t j = 0; j < size; j++)
    {
      float currDist = distance2(nodes[i]->n_center, vertices[j]);

      if (currDist > nodes[i]->n_radius)
      {
        nodes[i]->n_radius = currDist;
      }
    }

    nodes[i]->n_radius = sqrtf(nodes[i]->n_radius);
    nodes[i]->UpdateNode();
  }
}

void BVHTree::PCAMethod()
{
  for (size_t i = 0; i < nodes.size(); i++)
  {
    auto& vertices = nodes[i]->n_object->vertex;
    mat3 eigenValues = CovarianceMatrix(vertices, vertices.size(), nodes[i]->n_object->centroid);
    mat3 eigenVectors;

    Jacobi(eigenValues, eigenVectors);

    vec3 direction;
    int maxCoef = 0;
    float maxEiganVal = abs(eigenValues[0][0]);

    if (abs(eigenValues[1][1]) > maxEiganVal)
    {
      maxCoef = 1;
      maxEiganVal = abs(eigenValues[1][1]);;
    }
    if (abs(eigenValues[2][2]) > maxEiganVal)
    {
      maxCoef = 2;
      maxEiganVal = abs(eigenValues[2][2]);
    }

    direction[0] = eigenVectors[0][maxCoef];
    direction[1] = eigenVectors[1][maxCoef];
    direction[2] = eigenVectors[2][maxCoef];

    //find furthest points on direction
    float minVal = float(UINT32_MAX);
    vec3 minPoint;
    float maxVal = -float(UINT32_MAX);
    vec3 maxPoint;

    size_t size = vertices.size();
    for (size_t j = 0; j < size; j++)
    {
      float dotP = dot(direction, vertices[j]);

      if (dotP < minVal)
      {
        minPoint = vertices[j];
        minVal = dotP;
      }
      if (dotP > maxVal)
      {
        maxPoint = vertices[j];
        maxVal = dotP;
      }
    }

    nodes[i]->n_center = (maxPoint + minPoint) * 0.5f;
    nodes[i]->n_radius = length((maxPoint - minPoint) * 0.5f);

    for (size_t j = 0; j < size; j++)
    {
      float currDist = distance(nodes[i]->n_center, vertices[j]);

      if (currDist > nodes[i]->n_radius)
      {
        nodes[i]->n_radius = currDist;
      }
    }

    nodes[i]->UpdateNode();
  }
}

void BVHTree::CalcualteOBBs()
{
  for (size_t i = 0; i < nodes.size(); i++)
  {
    auto& vertices = nodes[i]->n_object->vertex;
    mat3 eigenValues = CovarianceMatrix(vertices, vertices.size(), nodes[i]->n_object->centroid);
    mat3 eigenVectors;

    Jacobi(eigenValues, eigenVectors);

    vec3 direction;
    int maxCoef = 0;
    float maxEiganVal = abs(eigenValues[0][0]);

    if (abs(eigenValues[1][1]) > maxEiganVal)
    {
      maxCoef = 1;
      maxEiganVal = abs(eigenValues[1][1]);;
    }
    if (abs(eigenValues[2][2]) > maxEiganVal)
    {
      maxCoef = 2;
      maxEiganVal = abs(eigenValues[2][2]);
    }

    direction[0] = (eigenValues[0][maxCoef]);
    direction[1] = (eigenValues[1][maxCoef]);
    direction[2] = (eigenValues[2][maxCoef]);

    size_t size = vertices.size();
    vec3 max(-FLT_MAX);
    vec3 min(FLT_MAX);
    for (size_t j = 0; j < size; j++)
    {
      float x = dot(eigenVectors[0], vertices[j]);
      max.x = glm::max(x, max.x);
      min.x = glm::min(x, min.x);

      float y = dot(eigenVectors[1], vertices[j]);
      max.y = glm::max(y, max.y);
      min.y = glm::min(y, min.y);

      float z = dot(eigenVectors[2], vertices[j]);
      max.z = glm::max(z, max.z);
      min.z = glm::min(z, min.z);
    }

    nodes[i]->n_obbCenter = (nodes[i]->n_max + nodes[i]->n_min) * 0.5f;
    nodes[i]->n_obbScale = (max - min) * 0.5f;

    nodes[i]->n_rotations = eigenVectors;
  }
}

void BVHTree::DrawDebug()
{
  SimpleScene_Quad* currScene = reinterpret_cast<SimpleScene_Quad*>(scene);

  glUseProgram(currScene->simple);
  Mesh* mesh = nullptr;

  if (volume == VolumeType::AABB || volume == VolumeType::OBB)
  {
    mesh = MeshManager::Instance()->GetMesh("cube2");
  }
  else
  {
    mesh = MeshManager::Instance()->GetMesh("sphereGen");
  }

  glBindVertexArray(mesh->VAO);

  GLint vTransformLoc = glGetUniformLocation(currScene->simple, "ViewTransform");
  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &camera.cam_viewMatrix[0][0]);
  }

  vTransformLoc = glGetUniformLocation(currScene->simple, "ProjTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &pers[0][0]);
  }

  auto curr = volumes.begin();
  auto end = volumes.end();

  while (curr != end)
  {
    vTransformLoc = glGetUniformLocation(currScene->simple, "modelColour");

    if (vTransformLoc >= 0)
    {
      glUniform3fv(vTransformLoc, 1, &GetColour(curr->first)[0]);
    }

    auto& vector = curr->second;
    size_t size = vector.size();
    for (size_t i = 0; i < size; ++i)
    {
      vTransformLoc = glGetUniformLocation(currScene->simple, "vertexTransform");
      if (vTransformLoc >= 0)
      {
        glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &vector[i][0][0]);
      }

      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDrawElements(GL_TRIANGLES, mesh->NumberOfElements, GL_UNSIGNED_INT, 0);
    }

    ++curr;
  }

  glUseProgram(0);
}

void BVHTree::ChangeTreeType(BVHType changeTo)
{
  if (type == changeTo)
  {
    return;
  }

  ClearTree();

  SimpleScene_Quad* quad = reinterpret_cast<SimpleScene_Quad*>(scene);
  if (changeTo == BVHType::BottomUP)
  {
    root = nullptr;
    BuildTreeBottomUp(quad->sceneMeshes);
    type = BVHType::BottomUP;
  }
  else
  {
    root = nullptr;
    BuildTreeTopDown(quad->meshPool, quad->meshNormals);
    type = BVHType::TopDown;
  }

  if (volume == VolumeType::OBB || volume == VolumeType::Ellipsoid)
  {
    CalcualteOBBs();
  }
}

void BVHTree::RebuildTreeTopDown()
{
  if (type == BVHType::BottomUP)
  {
    return;
  }

  ClearTree();

  SimpleScene_Quad* quad = reinterpret_cast<SimpleScene_Quad*>(scene);
  BuildTreeTopDown(quad->meshPool, quad->meshNormals);

  if (volume == VolumeType::OBB || volume == VolumeType::Ellipsoid)
  {
    CalcualteOBBs();
  }
}

void BVHTree::ClearTree()
{
  if (type == BVHType::BottomUP)
  {
    for (size_t i = 0; i < nodes.size(); i++)
    {
      Node* parent = nodes[i]->n_parent;

      parent->n_left = nodes[i] == parent->n_left ? nullptr : parent->n_left;
      parent->n_right = nodes[i] == parent->n_right ? nullptr : parent->n_right;

      nodes[i]->n_parent = nullptr;
      nodes[i] = nullptr;
    }
  }

  delete root;
  root = nullptr;
  nodes.clear();
}
