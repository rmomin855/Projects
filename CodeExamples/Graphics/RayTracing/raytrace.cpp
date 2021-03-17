//////////////////////////////////////////////////////////////////////
// Provides the framework for a raytracer.
////////////////////////////////////////////////////////////////////////

#include <vector>

#ifdef _WIN32
    // Includes for Windows
#include <windows.h>
#include <cstdlib>
#include <limits>
#include <crtdbg.h>
#else
    // Includes for Linux
#endif

#include "geom.h"
#include "raytrace.h"
#include "realtime.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// A good quality *thread-safe* Mersenne Twister random number generator.
#include <random>
std::random_device device;
std::mt19937_64 RNGen(device());
std::uniform_real_distribution<> myrandom(0.0, 1.0);
// Call myrandom(RNGen) to get a uniformly distributed random number in [0,1].

Scene::Scene()
{
#ifdef REAL
  realtime = new Realtime();
#endif // REAL

#ifndef REAL
  raytrace = new RayTrace();
#endif // !REAL
}

void Scene::Finit()
{
}

void Scene::triangleMesh(MeshData* mesh)
{
#ifdef REAL
  realtime->triangleMesh(mesh);
#endif // REAL

#ifndef REAL
  raytrace->triangleMesh(mesh);
#endif // !REAL
}

Quaternionf Orientation(int i,
  const std::vector<std::string>& strings,
  const std::vector<float>& f)
{
  Quaternionf q(1, 0, 0, 0); // Unit quaternion
  while (i < strings.size()) {
    std::string c = strings[i++];
    if (c == "x")
      q *= angleAxis(f[i++] * Radians, Vector3f::UnitX());
    else if (c == "y")
      q *= angleAxis(f[i++] * Radians, Vector3f::UnitY());
    else if (c == "z")
      q *= angleAxis(f[i++] * Radians, Vector3f::UnitZ());
    else if (c == "q") {
      q *= Quaternionf(f[i + 0], f[i + 1], f[i + 2], f[i + 3]);
      i += 4;
    }
    else if (c == "a") {
      q *= angleAxis(f[i + 0] * Radians, Vector3f(f[i + 1], f[i + 2], f[i + 3]).normalized());
      i += 4;
    }
  }
  return q;
}

////////////////////////////////////////////////////////////////////////
// Material: encapsulates surface properties
void Material::setTexture(const std::string path)
{
#ifdef REAL
  int width, height, n;
  stbi_set_flip_vertically_on_load(true);
  unsigned char* image = stbi_load(path.c_str(), &width, &height, &n, 0);

  // Realtime code below:  This sends the texture in *image to the graphics card.
  // The raytracer will not use this code (nor any features of OpenGL nor the graphics card).
  glGenTextures(1, &texid);
  glBindTexture(GL_TEXTURE_2D, texid);
  glTexImage2D(GL_TEXTURE_2D, 0, n, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 100);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(image);
#endif // REAL

}

void Scene::Command(const std::vector<std::string>& strings,
  const std::vector<float>& f)
{
  if (strings.size() == 0) return;
  std::string c = strings[0];

  if (c == "screen") {
    // syntax: screen width height
#ifdef REAL
    realtime->setScreen(int(f[1]), int(f[2]));
#endif // REAL

#ifndef REAL
    raytrace->setScreen(int(f[1]), int(f[2]));
#endif // !REAL

    width = int(f[1]);
    height = int(f[2]);
  }

  else if (c == "camera") {
    // syntax: camera x y z   ry   <orientation spec>
    // Eye position (x,y,z),  view orientation (qw qx qy qz),  frustum height ratio ry
#ifdef REAL
    realtime->setCamera(Vector3f(f[1], f[2], f[3]), Orientation(5, strings, f), f[4]);
#endif // REAL

#ifndef REAL
    raytrace->setCamera(Vector3f(f[1], f[2], f[3]), Orientation(5, strings, f), f[4]);
#endif // !REAL
  }

  else if (c == "ambient") {
    // syntax: ambient r g b
    // Sets the ambient color.  Note: This parameter is temporary.
    // It will be ignored once your raytracer becomes capable of
    // accurately *calculating* the true ambient light.
#ifdef REAL
    realtime->setAmbient(Vector3f(f[1], f[2], f[3]));
#endif // REAL

#ifndef REAL
    raytrace->setAmbient(Vector3f(f[1], f[2], f[3]));
#endif // !REAL
  }

  else if (c == "brdf") {
    // syntax: brdf  r g b   r g b  alpha
    // later:  brdf  r g b   r g b  alpha  r g b ior
    // First rgb is Diffuse reflection, second is specular reflection.
    // third is beer's law transmission followed by index of refraction.
    // Creates a Material instance to be picked up by successive shapes
    currentMat = new Material(Vector3f(f[1], f[2], f[3]), Vector3f(f[4], f[5], f[6]), f[7]);
  }

  else if (c == "light") {
    // syntax: light  r g b   
    // The rgb is the emission of the light
    // Creates a Material instance to be picked up by successive shapes
    currentMat = new Light(Vector3f(f[1], f[2], f[3]));
  }

  else if (c == "sphere") {
    // syntax: sphere x y z   r
    // Creates a Shape instance for a sphere defined by a center and radius
#ifdef REAL
    realtime->sphere(Vector3f(f[1], f[2], f[3]), f[4], currentMat);
#endif // REAL

#ifndef REAL
    raytrace->sphere(Vector3f(f[1], f[2], f[3]), f[4], currentMat);
#endif // !REAL
  }

  else if (c == "box") {
    // syntax: box bx by bz   dx dy dz
    // Creates a Shape instance for a box defined by a corner point and diagonal vector
#ifdef REAL
    realtime->box(Vector3f(f[1], f[2], f[3]), Vector3f(f[4], f[5], f[6]), currentMat);
#endif // REAL

#ifndef REAL
    raytrace->box(Vector3f(f[1], f[2], f[3]), Vector3f(f[4], f[5], f[6]), currentMat);
#endif // !REAL

  }

  else if (c == "cylinder") {
    // syntax: cylinder bx by bz   ax ay az  r
    // Creates a Shape instance for a cylinder defined by a base point, axis vector, and radius
#ifdef REAL
    realtime->cylinder(Vector3f(f[1], f[2], f[3]), Vector3f(f[4], f[5], f[6]), f[7], currentMat);
#endif // REAL

#ifndef REAL
    raytrace->cylinder(Vector3f(f[1], f[2], f[3]), Vector3f(f[4], f[5], f[6]), f[7], currentMat);
#endif // !REAL
  }


  else if (c == "mesh") {
    // syntax: mesh   filename   tx ty tz   s   <orientation>
    // Creates many Shape instances (one per triangle) by reading
    // model(s) from filename. All triangles are rotated by a
    // quaternion (qw qx qy qz), uniformly scaled by s, and
    // translated by (tx ty tz) .
    Matrix4f modelTr = translate(Vector3f(f[2], f[3], f[4]))
      * scale(Vector3f(f[5], f[5], f[5]))
      * toMat4(Orientation(6, strings, f));
    ReadAssimpFile(strings[1], modelTr);
  }


  else {
    fprintf(stderr, "\n*********************************************\n");
    fprintf(stderr, "* Unknown command: %s\n", c.c_str());
    fprintf(stderr, "*********************************************\n\n");
  }
}

void Scene::TraceImage(Color* image, const int pass)
{
#ifdef REAL
  realtime->run();                          // Remove this (realtime stuff)
#endif // REAL

  KdBVH<float, 3, Shape*> Tree(raytrace->shapes.begin(), raytrace->shapes.end());

  Vector3f X = raytrace->rx * raytrace->orient._transformVector(Vector3f::UnitX());
  Vector3f Y = raytrace->ry * raytrace->orient._transformVector(Vector3f::UnitY());
  Vector3f Z = -1.0f * raytrace->orient._transformVector(Vector3f::UnitZ());

  for (int i = 0; i < pass; i++)
  {
#pragma omp parallel for schedule(dynamic, 1) // Magic: Multi-thread y loop
    for (int y = 0; y < height; y++)
    {
      fprintf(stderr, "Rendering %4d\r", y);

      for (int x = 0; x < width; x++)
      {
        float dx = (2.0f * (x + 0.5f) / width) - 1.0f;
        float dy = (2.0f * (y + 0.5f) / height) - 1.0f;

        Vector3f dir = dx * X + dy * Y + Z;
        dir.normalize();
        Ray ray(raytrace->eye, dir);

        image[y * width + x] += raytrace->trace(ray, Tree);
      }
    }

    fprintf(stderr, "\n");
  }

  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      image[y * width + x] /= float(pass);
    }
  }
}

void RayTrace::sphere(const Vector3f center, const float r, Material* mat)
{
  Sphere* obj = new Sphere(center, r, mat);
  shapes.push_back(obj);
}

void RayTrace::box(const Vector3f base, const Vector3f diag, Material* mat)
{
  Box* obj = new Box(base, diag, mat);
  shapes.push_back(obj);
}

void RayTrace::cylinder(const Vector3f base, const Vector3f axis, const float radius, Material* mat)
{
  Cylinder* shape = new Cylinder(base, axis, radius, mat);
  shapes.push_back(shape);
}

void RayTrace::triangleMesh(MeshData* meshdata)
{
  size_t size = meshdata->triangles.size();

  for (size_t i = 0; i < size; i++)
  {
    TriData& tri = meshdata->triangles[i];
    VertexData v0 = meshdata->vertices[tri[0]];
    VertexData v1 = meshdata->vertices[tri[1]];
    VertexData v2 = meshdata->vertices[tri[2]];

    Triangle* obj = new Triangle(v0.pnt, v1.pnt, v2.pnt, v0.nrm, v1.nrm, v2.nrm, meshdata->mat);
    shapes.push_back(obj);
  }

  //if (meshdata->mat->isLight())
  //  lights.push_back(obj);
}

RayTrace::RayTrace()
{}

Color RayTrace::trace(Ray& ray, KdBVH<float, 3, Shape*>& tree)
{
  Minimizer mini(ray);
  float minDist = BVMinimize(tree, mini);

  //diffuse
  return mini.shape->mat->Kd;

  //distance
  return Vector3f(1.0f, 1.0f, 1.0f) * (minDist - 5.0f) / 4.0f;

  //normal
  for (size_t i = 0; i < 3; i++)
  {
    mini.minIntersect.N[i] = std::abs(mini.minIntersect.N[i]);
  }
  return mini.minIntersect.N;

  //Shape* shape = nullptr;
  //float t = FLT_MAX;

  //for (size_t i = 0; i < shapes.size(); i++)
  //{
  //  Intersection data;

  //  if (shapes[i]->intersect(ray, data))
  //  {
  //    if (!shape || data.t < t)
  //    {
  //      shape = shapes[i];
  //      t = data.t;
  //    }
  //  }
  //}

  //return shape->mat->Kd;
}

bool Sphere::intersect(const Ray& ray, Intersection& intersect)
{
  Vector3f _Q = ray.Q - C;

  float a = ray.D.dot(ray.D);
  float b = 2.0f * _Q.dot(ray.D);
  float c = _Q.dot(_Q) - (r * r);

  float determinant = (b * b) - (4.0f * a * c);

  if (determinant < 0)
  {
    return false;
  }

  float sqrtDet = sqrtf(determinant);

  float t1 = (-b + sqrtDet) / (2.0f * a);
  float t2 = (-b - sqrtDet) / (2.0f * a);

  if (t1 < 0.0f && t2 < 0.0f)
  {
    return false;
  }

  intersect.t = std::fmin(t1, t2);
  intersect.object = this;
  intersect.P = ray.eval(intersect.t);
  intersect.N = intersect.P - C;
  intersect.N.normalize();

  return true;
}

bool Triangle::intersect(const Ray& ray, Intersection& intersect)
{
  Vector3f e1 = V1 - V0;
  Vector3f e2 = V2 - V0;

  Vector3f p = ray.D.cross(e2);
  float d = p.dot(e1);

  if (d == 0.0)
  {
    return false;
  }

  Vector3f S = ray.Q - V0;
  float u = p.dot(S) / d;

  if (u < 0.0f || u > 1.0f)
  {
    return false;
  }

  Vector3f q = S.cross(e1);
  float v = ray.D.dot(q) / d;

  if (v < 0.0f || (u + v) > 1.0f)
  {
    return false;
  }

  float t = e2.dot(q) / d;

  if (t < 0.0f)
  {
    return false;
  }

  intersect.t = t;
  intersect.object = this;
  intersect.P = ray.eval(t);
  intersect.N = ((1.0f - u - v) * N0) + (u * N1) + (v * N2);
  intersect.N.normalize();

  return true;
}

bool Box::intersect(const Ray& ray, Intersection& intersect)
{
  Interval interval;

  for (size_t i = 0; i < 3; i++)
  {
    Interval curr = Interval::intersectSlab(ray, slabs[i]);

    if (curr.t1 == -1.0f)
    {
      continue;
    }

    if (interval.t0 < curr.t0)
    {
      interval.t0 = curr.t0;
      interval.n0 = curr.n0;
    }

    if (interval.t1 > curr.t1)
    {
      interval.t1 = curr.t1;
      interval.n1 = curr.n1;
    }
  }

  if (interval.t0 > interval.t1)
  {
    return false;
  }

  intersect.t = std::fmin(interval.t0, interval.t1);
  intersect.object = this;
  intersect.P = ray.eval(intersect.t);
  intersect.N = interval.n1;
  intersect.N.normalize();

  return true;
}

void Interval::intersect(Interval& other)
{
  if (t0 < other.t0)
  {
    t0 = other.t0;
    n0 = other.n0;
  }
  if (t1 > other.t1)
  {
    t1 = other.t1;
    n1 = other.n1;
  }
}

Interval Interval::intersectSlab(const Ray& ray, Slab& slab)
{
  float NDotD = slab.n.dot(ray.D);
  float NDotQ = slab.n.dot(ray.Q);

  if (NDotD != 0.0f)
  {
    float t0 = -(slab.d0 + NDotQ) / NDotD;
    float t1 = -(slab.d1 + NDotQ) / NDotD;

    if (t1 < t0)
    {
      std::swap(t0, t1);
    }

    return Interval(t0, t1, slab.n, slab.n);
  }
  else
  {
    float s0 = NDotQ + slab.d0;
    float s1 = NDotQ + slab.d1;

    if ((s0 > 0.0f && s1 < 0.0f) || (s0 < 0.0f && s1 > 0.0f))
    {
      return Interval();
    }
  }

  return Interval(0.0f, -1.0f);
}

bool Cylinder::intersect(const Ray& ray, Intersection& intersect)
{
  Quaternionf q = Quaternionf::FromTwoVectors(A, Vector3f::UnitZ());
  Vector3f Q_ = q._transformVector(ray.Q - B);
  Vector3f D_ = q._transformVector(ray.D);
  Ray transformed(Q_, D_);

  Interval interval1(0.0f, FLT_MAX);

  Slab slab2(Vector3f(0.0f, 0.0f, 1.0f), 0.0f, -A.norm());
  Interval interval2 = Interval::intersectSlab(transformed, slab2);

  const float a = (D_[0] * D_[0]) + (D_[1] * D_[1]);
  const float b = 2.0f * ((D_[0] * Q_[0]) + (D_[1] * Q_[1]));
  const float c = (Q_[0] * Q_[0]) + (Q_[1] * Q_[1]) - (r * r);

  float determinant = (b * b) - (4.0f * a * c);

  if (determinant < 0)
  {
    return false;
  }

  float sqrtDet = sqrtf(determinant);

  float t0 = (-b - sqrtDet) / (2.0f * a);
  float t1 = (-b + sqrtDet) / (2.0f * a);
  Interval interval3(t0, t1, transformed.eval(t0), transformed.eval(t1));
  interval3.n0[2] = 0.0f;
  interval3.n1[2] = 0.0f;

  interval1.intersect(interval2);
  interval1.intersect(interval3);

  t0 = interval1.t0;
  t1 = interval1.t1;

  if (t0 > t1)
  {
    return false;
  }

  if ((t0 == FLT_MAX && t1 == FLT_MAX) || (t0 < 0.0f && t1 < 0.0f))
  {
    return false;
  }

  intersect.t = t0 < t1 ? t0 : t1;
  intersect.object = this;
  intersect.P = ray.eval(intersect.t);
  intersect.N = t0 < t1 ? interval1.n0 : interval1.n1;
  intersect.N = q.conjugate()._transformVector(intersect.N);
  intersect.N.normalize();

  return true;
}

Bbox bounding_box(const Shape* obj)
{
  return obj->bbox(); // Assuming each Shape object has its own bbox method.
}

float Minimizer::minimumOnObject(Shape* obj)
{
  Intersection intersect;

  if (obj->intersect(ray, intersect))
  {
    if (!shape || intersect.t < minIntersect.t)
    {
      shape = intersect.object;
      minIntersect = intersect;
    }

    return intersect.t;
  }

  return INF;
}

float Minimizer::minimumOnVolume(const Bbox& box)
{
  Vector3f base = box.min(); // Box corner
  Vector3f diag = box.max() - base; // Box corner

  Slab slabs[3] = {};
  for (size_t i = 0; i < 3; i++)
  {
    slabs[i].n = Vector3f::Zero();
    slabs[i].n[i] = 1.0f;
    slabs[i].d0 = -base[i];
    slabs[i].d1 = -base[i] - diag[i];
  }

  Interval interval;

  for (size_t i = 0; i < 3; i++)
  {
    Interval curr = Interval::intersectSlab(ray, slabs[i]);

    if (curr.t1 == -1.0f)
    {
      continue;
    }

    if (interval.t0 < curr.t0)
    {
      interval.t0 = curr.t0;
      interval.n0 = curr.n0;
    }

    if (interval.t1 > curr.t1)
    {
      interval.t1 = curr.t1;
      interval.n1 = curr.n1;
    }
  }

  if (interval.t0 > interval.t1)
  {
    return INF;
  }

  return std::fmin(interval.t0, interval.t1);
}