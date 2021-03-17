#pragma once
#ifndef JACOBI
#define JACOBI
#include <glm/glm.hpp>
#include <vector>

using namespace glm;

mat3 CovarianceMatrix(std::vector<vec3>& pt, int numPts)
{
  mat3 cov;
  float xx, yy, zz, xy, xz, yz;
  xx = yy = zz = xy = xz = yz = 0.0f;

  // Compute covariance elements
  for (int i = 0; i < numPts; i++)
  {
    // Compute covariance of points
    xx += pt[i].x * pt[i].x;
    yy += pt[i].y * pt[i].y;
    zz += pt[i].z * pt[i].z;
    xy += pt[i].x * pt[i].y;
    xz += pt[i].x * pt[i].z;
    yz += pt[i].y * pt[i].z;
  }
  // Fill in the covariance matrix elements
  cov[0][0] = xx / numPts;
  cov[1][1] = yy / numPts;
  cov[2][2] = zz / numPts;
  cov[0][1] = cov[1][0] = xy / numPts;
  cov[0][2] = cov[2][0] = xz / numPts;
  cov[1][2] = cov[2][1] = yz / numPts;

  return cov;
}

inline void SymSchur2(mat3& a, int p, int q, float& c, float& s)
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

  // Repeat for some maximum number of iterations
  const int MAX_ITERATIONS = 50;
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
    J = mat3(1.0f);

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

#endif // !JACOBI
