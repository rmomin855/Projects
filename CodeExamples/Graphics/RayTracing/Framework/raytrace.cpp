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

#define MINPEN 0.0001f
#define ag 0.1
#define ap 2000

#define PHONG
//#define GGX

bool Material::CharacteristicX(float val)
{
  return val > 0.0f;
}

Color Material::F(float d)
{
  Color Ks = this->Ks;
  return Ks + (1.0f - Ks) * pow(1.0f - d, 5);
}

float Material::D(Vector3f m, Vector3f N)
{
  const float mdotN = m.dot(N);
  if (!CharacteristicX(mdotN)) return 0.0f;

  //phong
#ifdef PHONG
  float val = (ap + 2.0) / (2.0 * PI);
  return val * pow(mdotN, ap);
#endif // PHONG


  //ggx
#ifdef GGX
  const float tanm = std::sqrt(1.0f - std::pow(mdotN, 2)) / mdotN;

  float ag2 = ag * ag;
  const float ldenom = PI * std::pow(N.dot(m), 4);
  const float rdenom = std::pow(ag2 + tanm * tanm, 2);

  return ag2 / (ldenom * rdenom);
#endif // GGX

}

float Material::G(Vector3f Wi, Vector3f Wo, Vector3f m, Vector3f N)
{
  return G1(Wi, m, N) * G1(Wo, m, N);
}

float Material::G1(Vector3f v, Vector3f m, Vector3f N)
{
  const float vDotN = v.dot(N);
  if (vDotN > 1.0f) return 1.0f;

  if (!CharacteristicX(v.dot(m) / vDotN)) return 0.0f;

  const float tanv = std::sqrt(1.0f - (vDotN * vDotN)) / vDotN;
  if (tanv == 0.0f) return 1.0f;

  //phong
#ifdef PHONG
  float a = sqrt((ap / 2.0) + 1.0) / tanv;
  if (a < 1.6)
  {
    float num = 3.535 * a + (2.181 * a * a);
    float denom = 1.0 + 2.276 * a + (2.577 * a * a);
    return num / denom;
  }

  return 1.0f;
#endif // PHONG


  //ggx
#ifdef GGX
  float ag2 = ag * ag;
  return 2.f / (1 + std::sqrt(1 + ag2 * tanv * tanv));
#endif // GGX

}


float GeometryFactor(Intersection& A, Intersection& B)
{
  Vector3f D = A.P - B.P;

  float val = A.N.dot(D) * B.N.dot(D);
  val /= (D.dot(D) * D.dot(D));

  val = std::abs(val);
  return val;
}

Vector3f SampleLobe(Vector3f N, float c, float phi)
{
  float s = sqrt(1.0f - (c * c));
  Vector3f K = Vector3f(s * cosf(phi), s * sin(phi), c);
  Quaternionf q = Quaternionf::FromTwoVectors(Vector3f::UnitZ(), N);
  return q._transformVector(K);
}

Intersection SampleSphere(Vector3f C, float R, Shape* sphere)
{
  float rand1 = myrandom(RNGen);
  float rand2 = myrandom(RNGen);

  float z = (2.0f * rand1) - 1.0f;
  float r = sqrt(1.0f - (z * z));
  float a = 2.0f * PI * rand2;

  Intersection record;
  record.N = Vector3f(r * cos(a), r * sin(a), z).normalized();
  record.P = C + (R * record.N);
  record.object = sphere;
  return record;
}

Vector3f Material::SampleBrdf(Vector3f Wo, Vector3f N)
{
  float choose = myrandom(RNGen);
  float rand1 = myrandom(RNGen);
  float rand2 = myrandom(RNGen);

  //diffuse
  if (choose < Pd)
  {
    return SampleLobe(N, sqrt(rand1), 2.0 * PI * rand2);
  }

  //reflection
  float cosm;

  //ggx
#ifdef GGX
  float num = ag * std::sqrt(rand1);
  float denom = std::sqrt(1.0 - rand1);
  cosm = cos(atan(num / denom));
#endif // GGX


  //phong
#ifdef PHONG
  cosm = pow(rand1, 1.0 / (ap + 1.0));
#endif // PHONG

  Vector3f m = SampleLobe(N, cosm, 2.0 * PI * rand2);
  return (2.0 * Wo.dot(m) * m) - Wo;
}

float Material::PdfBrdf(Vector3f Wo, Vector3f N, Vector3f Wi)
{
  //float val = N.dot(Wi);
  //val = std::abs(val);
  //return val / PI;

  // // //
  float pd = abs(Wi.dot(N)) / PI;

  Vector3f m = (Wo + Wi).normalized();

  if (m == Vector3f::Zero())
  {
    return 0.0f;
  }

  float pr = D(m, N) * abs(m.dot(N));
  pr *= 1.0 / (4.0 * abs(Wi.dot(m)));

  return (pd * Pd) + (pr * Pr);
}

Color Material::EvalScattering(Vector3f Wo, Vector3f N, Vector3f Wi)
{
  //float val = N.dot(Wi);
  //val = std::abs(val);
  //return val * Kd / PI;

  // // //
  Color Ed = Kd / PI;

  Vector3f m = (Wi + Wo).normalized();

  float WidotN = std::abs(Wi.dot(N));
  float WodotN = std::abs(Wo.dot(N));

  float d = D(m, N);
  float g = G(Wi, Wo, m, N);
  Color f = F(abs(Wi.dot(m)));

  Color Er = (f * g * d) / (4 * WidotN * WodotN);

  return WidotN * (Ed + Er);
}

Intersection RayTrace::SampleLight()
{
  float rand = myrandom(RNGen);
  size_t index = size_t(rand * lights.size());

  if (rand == 1.0f)
  {
    --index;
  }

  Sphere* light = reinterpret_cast<Sphere*>(lights[index]);

  return SampleSphere(light->C, light->r, light);
}

float RayTrace::PdfLight(Intersection& Q)
{
  Sphere* sphere = reinterpret_cast<Sphere*>(Q.object);
  float area = 4.0f * PI * sphere->r * sphere->r;
  return 1.0f / (area * lights.size());
}

Color RayTrace::EvalRadiance(Intersection& Q)
{
  return Q.object->mat->Kd;
}

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
  Tree = KdBVH<float, 3, Shape*>(raytrace->shapes.begin(), raytrace->shapes.end());
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

int fCount = 0;
int CCount = 0;

void Scene::TraceImage(Color* image)
{
#ifdef REAL
  realtime->run();                          // Remove this (realtime stuff)
#endif // REAL

#ifndef REAL
  Vector3f X = raytrace->rx * raytrace->orient._transformVector(Vector3f::UnitX());
  Vector3f Y = raytrace->ry * raytrace->orient._transformVector(Vector3f::UnitY());
  Vector3f Z = -1.0f * raytrace->orient._transformVector(Vector3f::UnitZ());

#pragma omp parallel for schedule(dynamic, 1) // Magic: Multi-thread y loop
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      float dx = (2.0f * (x + myrandom(RNGen)) / width) - 1.0f;
      float dy = (2.0f * (y + myrandom(RNGen)) / height) - 1.0f;

      Vector3f dir = dx * X + dy * Y + Z;
      dir.normalize();
      Ray ray(raytrace->eye, dir);

      Color C;
      Vector3f c;

      C = raytrace->trace(ray, Tree);
      c = C;

      if (isnan(C.x()) || isnan(C.y()) || isnan(C.z()) || c.norm() > sqrt(75))
      {
        continue;
      }

      image[y * width + x] += C * 2.0f;
    }
  }
#endif
}

void RayTrace::sphere(const Vector3f center, const float r, Material* mat)
{
  Sphere* obj = new Sphere(center, r, mat);
  shapes.push_back(obj);

  if (mat->isLight())
  {
    lights.push_back(obj);
  }
}

void RayTrace::box(const Vector3f base, const Vector3f diag, Material* mat)
{
  Box* obj = new Box(base, diag, mat);
  shapes.push_back(obj);

  if (mat->isLight())
  {
    lights.push_back(obj);
  }
}

void RayTrace::cylinder(const Vector3f base, const Vector3f axis, const float radius, Material* mat)
{
  Cylinder* shape = new Cylinder(base, axis, radius, mat);
  shapes.push_back(shape);

  if (mat->isLight())
  {
    lights.push_back(shape);
  }
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

    if (meshdata->mat->isLight())
    {
      lights.push_back(obj);
    }
  }

  //if (meshdata->mat->isLight())
  //  lights.push_back(obj);
}

RayTrace::RayTrace()
{}

Intersection RayTrace::CastRay(Ray& ray, KdBVH<float, 3, Shape*>& tree)
{
  Minimizer mini(ray);
  BVMinimize(tree, mini);
  return mini.record;
}

Color RayTrace::trace(Ray& ray, KdBVH<float, 3, Shape*>& tree)
{
  Color C = Color(0.0f, 0.0f, 0.0f);
  Color W = Color(1.0f, 1.0f, 1.0f);

  Intersection P = CastRay(ray, tree);
  Vector3f N = P.N;

  if (P.object == nullptr)
  {
    return C;
  }

  if (P.object->mat->isLight())
  {
    return EvalRadiance(P);
  }

  Vector3f Wo = -ray.D;

  float RussianRoulette = 0.8f;
  while (myrandom(RNGen) <= RussianRoulette)
  {
    Vector3f Wi;
    float p;

    //Explict Light Connection
    ////////////////////////////////////////////////////////
    Intersection L = SampleLight();
    p = PdfLight(L) / GeometryFactor(P, L);
    Wi = L.P - P.P;
    Wi.normalize();

    Intersection I = CastRay(Ray(P.P, Wi), tree);

    if (p > 0.000001f && I.object && I.object == L.object)
    {
      Color f = P.object->mat->EvalScattering(Wo, N, Wi);
      C += 0.5f * W * (f / p) * EvalRadiance(L);
    }

    ////////////////////////////////////////////////////////

    //Implict Light Connection
    N = P.N;
    Wi = P.object->mat->SampleBrdf(Wo, N).normalized();

    Intersection Q = CastRay(Ray(P.P, Wi), tree);

    if (!Q.object)
    {
      break;
    }

    Color f = P.object->mat->EvalScattering(Wo, N, Wi);
    p = Q.object->mat->PdfBrdf(Wo, N, Wi);

    if (p < 0.000001)
    {
      break;
    }

    p *= RussianRoulette;

    W *= (f / p);

    if (Q.object->mat->isLight())
    {
      C += 0.5f * W * EvalRadiance(Q);
      //C += W * EvalRadiance(Q);
      break;
    }

    P = Q;
    Wo = -Wi;
  }

  return C;
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

  if (t1 < MINPEN && t2 < MINPEN)
  {
    return false;
  }

  intersect.t = std::fmin(t1, t2);

  if (intersect.t < MINPEN)
  {
    intersect.t = std::fmax(t1, t2);
  }

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

  if (t < MINPEN)
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

  if (interval.t0 < MINPEN && interval.t1 < MINPEN)
  {
    return false;
  }

  intersect.t = std::fmin(interval.t0, interval.t1);

  if (intersect.t < MINPEN)
  {
    intersect.t = std::fmax(interval.t0, interval.t1);
  }

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

  if ((t0 == FLT_MAX && t1 == FLT_MAX) || (t0 < MINPEN && t1 < MINPEN))
  {
    return false;
  }

  if (t0 > MINPEN)
  {
    intersect.t = t0;
    intersect.N = interval1.n0;
  }
  else
  {
    intersect.t = t1;
    intersect.N = interval1.n1;
  }

  intersect.object = this;
  intersect.P = ray.eval(intersect.t);
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
    if (!shape || intersect.t < record.t)
    {
      shape = intersect.object;
      record = intersect;
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