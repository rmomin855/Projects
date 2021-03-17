#pragma once
#include <glm/glm.hpp>
#include <set>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <ppl.h>

using namespace glm;

#define TOLERANCE 0.00001f

struct Face
{
  vec3 point1;
  vec3 point2;
  vec3 point3;

  bool operator==(const Face& rhs) const
  {
    return (point1 == rhs.point1 && point2 == rhs.point2 && point3 == rhs.point3);
  }
};

struct Edge
{
  vec3 point1;
  vec3 point2;

  bool operator==(const Edge& rhs) const
  {
    if (point1 == rhs.point1 && point2 == rhs.point2)
    {
      return true;
    }

    return false;
  }
};

inline float AreaOfTriangle(const vec3& point1, const vec3& point2, const vec3& point3)
{
  vec3 dir12 = point2 - point1;
  vec3 dir13 = point3 - point1;

  vec3 crossP = cross(dir12, dir13);

  return length(crossP) * 0.5f;
}

inline vec3 CenterOfTriangle(const vec3& point1, const vec3& point2, const vec3& point3)
{
  return (point1 + point2 + point3) / 3.0f;
}

inline vec3 TripleProduct(const vec3& vector1, const vec3& vector2, const vec3& vector3)
{
  return (vector2 * dot(vector1, vector3)) - (vector1 * dot(vector2, vector3));
}

inline void ConvertAntiClockwise(vec3& point1, vec3& point2, vec3& point3)
{
  vec3 dir12 = point2 - point1;
  vec3 dir13 = point3 - point1;
  vec3 center = CenterOfTriangle(point1, point2, point3);

  vec3 crossP = cross(dir12, dir13);
  vec3 dir12N = cross(crossP, dir13);

  if (dot(dir12N, center - point1) > 0.0f)
  {
    std::swap(point1, point3);
  }
}

inline bool OriginInFrontOfPlane(vec3& point1, vec3& point2, vec3& point3, vec3& insidePoint)
{
  vec3 dir12 = point2 - point1;
  vec3 dir13 = point3 - point1;
  vec3 center = CenterOfTriangle(point1, point2, point3);

  vec3 triCross = cross(dir12, dir13);

  if (dot(triCross, insidePoint - point1) >= 0.0f)
  {
    triCross = -triCross;
  }

  return (dot(triCross, -point1) >= 0.0f);
}

inline vec3 GetSupportPoint(std::vector<vec3>& points, vec3 dir)
{
  float max = -FLT_MAX;
  vec3 point = vec3(0.0f);

  size_t size = points.size();
  for (size_t i = 0; i < size; i++)
  {
    float dotVal = dot(dir, points[i]);

    if (dotVal > max)
    {
      point = points[i];
      max = dotVal;
    }
  }

  return point;
}

inline void GetMinKowskiDiff(std::vector<vec3>& difference, std::vector<vec3>& pointSet1, std::vector<vec3>& pointSet2)
{
  size_t size1 = pointSet1.size();
  size_t size2 = pointSet2.size();

  difference.reserve(size1 * size2);

  for (size_t i = 0; i < size1; i++)
  {
    for (size_t j = 0; j < size2; j++)
    {
      difference.push_back(pointSet1[i] - pointSet2[j]);
    }
  }
}

inline vec3 GetAverage(std::vector<vec3>& points)
{
  vec3 average = vec3(0.0f);

  size_t size = points.size();
  for (size_t i = 0; i < size; i++)
  {
    average += points[i];
  }
  average /= size;

  return average;
}

inline void TransformPoints(std::vector<vec3>& points, const mat4& matrix)
{
  size_t size = points.size();
  concurrency::parallel_for(size_t(0), size, [&](size_t i)
    {
      points[i] = matrix * vec4(points[i], 1.0f);
    });

  //for (size_t i = 0; i < size; i++)
  //{
  //  points[i] = matrix * vec4(points[i], 1.0f);
  //}
}

inline vec3 GetSupportDifference(std::vector<vec3>& pointSet1, std::vector<vec3>& pointSet2, vec3 dir)
{
  vec3 support1 = GetSupportPoint(pointSet1, dir);
  vec3 support2 = GetSupportPoint(pointSet2, -dir);

  return support1 - support2;
}

inline vec3 GetClosestPointOnLine(vec3 point1, vec3 point2, std::vector<vec3>& reducedSimplex)
{
  reducedSimplex.clear();
  vec3 dir = point2 - point1;

  //if origin is behind point1
  if (dot(dir, -point1) <= 0.0f)
  {
    reducedSimplex.push_back(point1);
    return point1;
  }

  //if origin is behind point2
  if (dot(-dir, -point2) <= 0.0f)
  {
    reducedSimplex.push_back(point2);
    return point2;
  }

  dir = normalize(dir);

  reducedSimplex.push_back(point1);
  reducedSimplex.push_back(point2);

  //get ratio over whole vector
  float mag = dot(dir, -point1);
  //get point on line
  vec3 point = point1 + (mag * dir);
  return point;
}

inline vec3 GetClosestPointOnTriangle(vec3 point1, vec3 point2, vec3 point3, std::vector<vec3>& reducedSimplex)
{
  reducedSimplex.clear();

  vec3 center = CenterOfTriangle(point1, point2, point3);
  vec3 dir12 = point2 - point1;
  vec3 dir13 = point3 - point1;
  vec3 dir23 = point3 - point2;

  //voronoi region of point 1
  if ((dot(dir12, -point1) <= 0.0f) && (dot(dir13, -point1) <= 0.0f))
  {
    reducedSimplex.push_back(point1);
    return point1;
  }
  //voronoi region of point 2
  if ((dot(-dir12, -point2) <= 0.0f) && (dot(dir23, -point2) <= 0.0f))
  {
    reducedSimplex.push_back(point2);
    return point2;
  }

  //voronoi region of point 3
  if ((dot(-dir13, -point3) <= 0.0f) && (dot(-dir23, -point3) <= 0.0f))
  {
    reducedSimplex.push_back(point3);
    return point3;
  }

  vec3 triNormal = cross(dir12, dir13);

  //voronoi region of edge 1-2
  vec3 dir12N = cross(triNormal, dir12);
  if (dot(dir12N, center - point1) > 0.0f)
  {
    dir12N = -dir12N;
  }

  if (dot(dir12N, -point1) >= 0.0f)
  {
    return GetClosestPointOnLine(point1, point2, reducedSimplex);
  }

  //voronoi region of edge 2-3
  vec3 dir23N = cross(triNormal, dir23);
  if (dot(dir23N, center - point2) > 0.0f)
  {
    dir23N = -dir23N;
  }

  if (dot(dir23N, -point2) >= 0.0f)
  {
    return GetClosestPointOnLine(point2, point3, reducedSimplex);
  }

  //voronoi region of edge 3-2
  vec3 dir31N = cross(triNormal, -dir13);
  if (dot(dir31N, center - point3) > 0.0f)
  {
    dir31N = -dir31N;
  }

  if (dot(dir31N, -point3) >= 0.0f)
  {
    return GetClosestPointOnLine(point3, point1, reducedSimplex);
  }

  //point on triangle, barycentric co-ordinates
  float areaOfTriangle = AreaOfTriangle(point1, point2, point3);

  float areaOfP1Triangle = AreaOfTriangle(vec3(0.0f), point2, point3) / areaOfTriangle;
  float areaOfP2Triangle = AreaOfTriangle(point1, vec3(0.0f), point3) / areaOfTriangle;
  float areaOfP3Triangle = AreaOfTriangle(point1, point2, vec3(0.0f)) / areaOfTriangle;

  reducedSimplex.push_back(point1);
  reducedSimplex.push_back(point2);
  reducedSimplex.push_back(point3);
  return (areaOfP1Triangle * point1) + (areaOfP2Triangle * point2) + (areaOfP3Triangle * point3);
}

inline vec3 GetClosestPointOnTetrahedron(vec3 point1, vec3 point2, vec3 point3, vec3 point4, std::vector<vec3>& reducedSimplex)
{
  vec3 average = (point1 + point2 + point3 + point4) / 4.0f;

  if (OriginInFrontOfPlane(point1, point2, point3, average))
  {
    return GetClosestPointOnTriangle(point1, point2, point3, reducedSimplex);
  }

  if (OriginInFrontOfPlane(point1, point3, point4, average))
  {
    return GetClosestPointOnTriangle(point1, point3, point4, reducedSimplex);
  }

  if (OriginInFrontOfPlane(point1, point2, point4, average))
  {
    return GetClosestPointOnTriangle(point1, point2, point4, reducedSimplex);
  }

  if (OriginInFrontOfPlane(point2, point3, point4, average))
  {
    return GetClosestPointOnTriangle(point2, point3, point4, reducedSimplex);
  }

  return vec3(0.0f);
}

inline vec3 GetClosestPointToOrigin(std::vector<vec3>& simplex)
{
  switch (simplex.size())
  {
  case 2:
    return GetClosestPointOnLine(simplex[0], simplex[1], simplex);
    break;
  case 3:
    return GetClosestPointOnTriangle(simplex[0], simplex[1], simplex[2], simplex);
    break;
  case 4:
    return GetClosestPointOnTetrahedron(simplex[0], simplex[1], simplex[2], simplex[3], simplex);
    break;
  default:
    return simplex[0];
    break;
  }
}

inline void LessThanEpsilonToZero(vec3& point)
{
  if (std::abs(point.x) < TOLERANCE)
  {
    point.x = 0.0f;
  }

  if (std::abs(point.y) < TOLERANCE)
  {
    point.y = 0.0f;
  }

  if (std::abs(point.z) < TOLERANCE)
  {
    point.z = 0.0f;
  }
}

inline void FindClosestFace(std::vector<Face>& faces, vec3& FaceNormal, float& distance)
{
  float shortestDist = FLT_MAX;
  size_t index = 0;

  size_t size = faces.size();

  for (size_t i = 0; i < size; i++)
  {
    vec3 dir12 = faces[i].point2 - faces[i].point1;
    vec3 dir13 = faces[i].point3 - faces[i].point1;

    vec3 normal = cross(dir12, dir13);
    normal = normalize(normal);
    float currDist = dot(normal, -faces[i].point1);

    if (currDist < 0.0f)
    {
      currDist = -currDist;
      normal = -normal;
    }

    if (currDist < shortestDist)
    {
      shortestDist = currDist;
      FaceNormal = normal;
      distance = currDist;
      index = i;
    }
  }
}

inline void ReconstructFaces(std::vector<Face>& faces, vec3 supportPoint)
{
  std::vector<Face> removedFaces;
  size_t size = faces.size();

  //get faces "seen" by the new point
  for (size_t i = 0; i < size; i++)
  {
    Face& currFace = faces[i];
    vec3 dir12 = currFace.point2 - currFace.point1;
    vec3 dir13 = currFace.point3 - currFace.point1;
    vec3 triNormal = cross(dir12, dir13);

    if (dot(triNormal, currFace.point1) > 0.0f)
    {
      triNormal = -triNormal;
    }

    if (dot(triNormal, supportPoint - currFace.point1) > 0.0f)
    {
      removedFaces.push_back(currFace);
    }
  }

  std::vector<Edge> removedEdges;
  std::vector<Edge> edgesToDelete;
  size = removedFaces.size();
  //edges that need to be deleted for good
  for (size_t i = 0; i < size; i++)
  {
    Face currFace = removedFaces[i];

    Edge triEdges[3] =
    { { currFace.point1, currFace.point2 },
      { currFace.point2, currFace.point3 },
      { currFace.point3, currFace.point1 } };

    for (size_t e = 0; e < 3; e++)
    {
      auto index = std::find(removedEdges.begin(), removedEdges.end(), triEdges[e]);

      if (index == removedEdges.end())
      {
        removedEdges.push_back(triEdges[e]);
      }
      else
      {
        edgesToDelete.push_back(triEdges[e]);
      }
    }
  }

  size = edgesToDelete.size();
  for (size_t i = 0; i < size; i++)
  {
    auto index = std::find(removedEdges.begin(), removedEdges.end(), edgesToDelete[i]);
    if (index != removedEdges.end())
    {
      removedEdges.erase(index);
    }
  }

  size = removedFaces.size();
  //remove faces
  for (size_t i = 0; i < size; i++)
  {
    auto index = std::find(faces.begin(), faces.end(), removedFaces[i]);
    faces.erase(index);
  }

  size = removedEdges.size();
  //add faces with new point included
  for (size_t i = 0; i < size; i++)
  {
    faces.push_back({ removedEdges[i].point1, removedEdges[i].point2, supportPoint });
  }
}

inline void CalculateTangents(glm::vec3 normal, ContactManifold& manifold)
{
  // find least axis of least significant component
  const float absX = std::fabs(normal.x);
  const float absY = std::fabs(normal.y);
  const float absZ = std::fabs(normal.z);
  vec3 axis(0.0f, 0.0f, 0.0f);
  if (absX > absY)
  {
    if (absX > absZ)
      axis.x = 1.0f; // X > Y > Z, X > Z > Y
    else
      axis.z = 1.0f; // Z > X > Y
  }
  else
  {
    if (absY > absZ)
      axis.y = 1.0f; // Y > X > Z, Y > Z > X
    else
      axis.z = 1.0f; // Z > Y > X
  }

  // compute tangents
  manifold.cp_tangent1 = cross(normal, axis);
  manifold.cp_tangent2 = cross(normal, manifold.cp_tangent1);
}