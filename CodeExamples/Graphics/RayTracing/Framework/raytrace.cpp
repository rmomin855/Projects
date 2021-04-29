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
//std::random_device device;
//std::mt19937_64 RNGen(device());
std::mt19937_64 RNGen;
std::uniform_real_distribution<> myrandom(0.0, 1.0);
// Call myrandom(RNGen) to get a uniformly distributed random number in [0,1].

#define MINPEN 0.0001f

#define ag 0.1f
#define ap 10000
#define ab 0.001f

float sign(float r)
{
  return (r >= 0) ? 1 : -1;
}

bool IsCorrupt(float val)
{
  return isnan(val) || isinf(val);
}

bool IsCorrupt(Vector3f vec)
{
  return IsCorrupt(vec.x()) || IsCorrupt(vec.y()) || IsCorrupt(vec.z());
}

float ChiX(float d)
{
  return d > 0.0f ? 1.0f : 0.0f;
}

Color A(const Vector3f& Wo, Intersection& i)
{
  if (Wo.dot(i.N) < 0.0f)
  {
    float r = exp(i.t * log(i.object->mat->Kt.x()));
    float g = exp(i.t * log(i.object->mat->Kt.y()));
    float b = exp(i.t * log(i.object->mat->Kt.z()));
    return Color(r, g, b);
  }

  return Color(1.0f, 1.0f, 1.0f);
}

Color F(float LdotH, Intersection& i)
{
  Color Ks = i.object->mat->Ks;
  return Ks + (1.0f - Ks) * pow((1.0f - abs(LdotH)), 5);;
}

float D(Vector3f m, Intersection& i)
{
  if (!ChiX(m.dot(i.N)))
  {
    return 0.0f;
  }

  float num = ap + 2.0f;
  float denom = 2.0f * PI;

  return (num / denom) * pow(m.dot(i.N), ap);
}

float G1(Vector3f v, Vector3f m, Intersection& i)
{
  float VN = v.dot(i.N);
  float VM = v.dot(m);

  if (!ChiX(VM / VN))
  {
    return 0.0f;
  }

  float tanv = sqrt(1.0f - (VN * VN)) / VN;

  if (VN > 1.0f)
  {
    return 1.0f;
  }

  if (!tanv)
  {
    return 1.0f;
  }

  float a = sqrt((ap / 2.0f) + 1.0f) / tanv;

  float num = 3.535f * a + 2.181f * a * a;
  float denom = 1.0f + 2.276f * a + 2.577f * a * a;
  return num / denom;
}

float G(const Vector3f& Wo, const Vector3f& m, const Vector3f& Wi, Intersection& i)
{
  return G1(Wi, m, i) * G1(Wo, m, i);
}

float GeometryFactor(Intersection A, Intersection B)
{
  Vector3f D = A.P - B.P;
  float DD = D.dot(D);
  float AD = A.N.dot(D);
  float BD = B.N.dot(D);

  return abs((AD * BD) / (DD * DD));
}

Vector3f SampleLobe(Vector3f N, float c, float phi)
{
  float s1 = sqrt(1 - c * c);
  Vector3f K(s1 * cos(phi), s1 * sin(phi), c); // Vector centered around Z-axis
  Quaternionf q = Quaternionf::FromTwoVectors(Vector3f::UnitZ(), N); // q rotates Z to N
  return q._transformVector(K); // K rotated to N's frame
}

Intersection SampleSphere(Vector3f C, float R, Shape* shape)
{
  float rand1 = myrandom(RNGen);
  float rand2 = myrandom(RNGen);

  float z = (2.0f * rand1) - 1.0f;
  float r = sqrt(1.0f - (z * z));
  float a = 2.0f * PI * rand2;

  Intersection record;
  record.N = Vector3f(r * cos(a), r * sin(a), z).normalized();
  record.P = C + (R * record.N);
  record.object = shape;
  return record;
}

Vector3f SampleBrdf(Vector3f Wo, Intersection& i)
{
  float rand1 = myrandom(RNGen);
  float rand2 = myrandom(RNGen);

  float pd = i.object->mat->pd;
  float pr = i.object->mat->pr;
  float pt = i.object->mat->pt;

  float cosTheataM = pow(rand1, 1.0f / (ap + 1.0f));
  Vector3f m = SampleLobe(i.N, cosTheataM, 2.0f * PI * rand2);

  if (rand1 < pd)
  {
    return SampleLobe(i.N, sqrt(rand1), 2.0f * rand2 * PI);
  }
  else if (rand1 < (pd + pr))
  {
    return 2.0f * abs(Wo.dot(m)) * m - Wo;
  }

  // transmission prob
  float ni = 1.0f, no = 1.0f;

  if (Wo.dot(i.N) > 0.0f)
  {
    no = i.object->mat->IOR;
  }
  else
  {
    ni = i.object->mat->IOR;
  }

  float n = ni / no;

  // test radiance
  float r = 1.0f - (n * n) * (1.0f - (Wo.dot(m) * Wo.dot(m)));

  if (r < 0.0f)
  {
    return 2.0f * abs(Wo.dot(m)) * m - Wo;
  }

  return (n * Wo.dot(m) - sign(Wo.dot(i.N)) * sqrt(r)) * m - (n * Wo);
}

float PdfBrdf(Vector3f Wo, Vector3f Wi, Intersection& i)
{
  float pd = i.object->mat->pd;
  float pr = i.object->mat->pr;
  float pt = i.object->mat->pt;

  float Pd = std::abs(Wi.dot(i.N)) / PI;

  if (!pr && !pt)
  {
    return pd * Pd;
  }

  // specular reflection prob
  Vector3f m = (Wo + Wi).normalized();
  float end = (1.0f / (4.0f * std::abs(Wi.dot(m))));
  float Pr = D(m, i) * std::abs(m.dot(i.N)) * end;

  if (!pt)
  {
    return pd * Pd + pr * Pr;
  }

  // transmission prob
  float ni = 1.0f, no = 1.0f;
  if (Wo.dot(i.N) > 0.0f)
  {
    no = i.object->mat->IOR;
  }
  else
  {
    ni = i.object->mat->IOR;
  }

  float n = ni / no;

  m = -1.0f * (no * Wi + ni * Wo).normalized();

  // test radiance
  float r = 1.0f - (n * n) * (1.0f - (Wo.dot(m) * Wo.dot(m)));

  float Pt;
  // total internal reflection
  if (r < 0.0f)
  {
    Pt = Pr;
  }
  else
  {
    float num = (no * no) * std::abs(Wi.dot(m));
    float den = (no * Wi.dot(m) + ni * Wo.dot(m));

    float end2 = num / (den * den);
    Pt = D(m, i) * std::abs(m.dot(i.N)) * end2;
  }

  return pd * Pd + pr * Pr + pt * Pt;
}

Color EvalScattering(Vector3f Wo, Intersection i, Vector3f Wi)
{
  float pd = i.object->mat->pd;
  float pr = i.object->mat->pr;
  float pt = i.object->mat->pt;

  // diffuse term
  Color Ed = Color(i.object->mat->Kd) / PI;
  float LdotN = std::abs(Wi.dot(i.N));

  if (!pr && !pt)
  {
    return LdotN * Ed;
  }

  // specular term
  Vector3f m = (Wi + Wo).normalized();

  Color f = F(Wi.dot(m), i);
  float g = G(Wo, m, Wi, i);
  float d = D(m, i);
  float VdotN = std::abs(Wo.dot(i.N));

  Color Er = (f * g * d) / (4.0f * LdotN * VdotN);

  if (!pt)
  {
    return LdotN * (Ed + Er);
  }

  // transmission term
  float ni = 1.0f, no = 1.0f;
  if (Wo.dot(i.N) > 0.0f)
  {
    no = i.object->mat->IOR;
  }
  else
  {
    ni = i.object->mat->IOR;
  }

  float n = ni / no;

  m = -1.0f * (no * Wi + ni * Wo).normalized();

  // test radiance
  float r = 1.0f - (n * n) * (1.0f - (Wo.dot(m) * Wo.dot(m)));

  Color Et;

  // total internal reflection
  if (r < 0.0f)
  {
    Et = A(Wo, i) * Er;
  }
  else
  {
    f = 1.0f - F(Wi.dot(m), i);
    g = G(Wo, m, Wi, i);
    d = D(m, i);

    Color left = Color(g * d * f) / (LdotN * VdotN);

    float rightNum = std::abs(Wi.dot(m)) * std::abs(Wo.dot(m)) * no * no;
    float rightDen = (no * Wi.dot(m) + ni * Wo.dot(m));

    Et = A(Wo, i) * left * (rightNum / (rightDen * rightDen));
  }

  return LdotN * (Ed + Er + Et);
}

Intersection RayTrace::SampleLight()
{
  float rand = myrandom(RNGen);
  size_t index = size_t(rand * lights.size());
  Intersection inter;

  if (rand == 1.0f)
  {
    --index;
  }

  if (dynamic_cast<Sphere*>(lights[index]))
  {
    Sphere* light = reinterpret_cast<Sphere*>(lights[index]);

    inter = SampleSphere(light->C, light->r, light);
  }
  if (dynamic_cast<Mesh*>(lights[index]))
  {
    Mesh* mesh = reinterpret_cast<Mesh*>(lights[index]);
    float rand = myrandom(RNGen);
    size_t triIndex = size_t(rand * mesh->triangles.size());

    if (rand == 1.0f)
    {
      --triIndex;
    }

    inter.P = mesh->triangles[triIndex]->V0;
    inter.N = mesh->triangles[triIndex]->N0;
    inter.object = lights[index];
  }
  if (dynamic_cast<Cylinder*>(lights[index]))
  {
    Cylinder* cylinder = reinterpret_cast<Cylinder*>(lights[index]);

    inter.object = lights[index];
    inter.P = cylinder->B;
    inter.N = cylinder->A.normalized();
  }
  if (dynamic_cast<Box*>(lights[index]))
  {
    Box* box = reinterpret_cast<Box*>(lights[index]);

    inter.object = lights[index];
    inter.P = box->base;
    inter.N = Vector3f(0.0, 0.0f, 1.0f);
  }

  return inter;
}

float RayTrace::PdfLight(Intersection& Q)
{
  float area = 0.0f;

  if (dynamic_cast<Sphere*>(Q.object))
  {
    float r = reinterpret_cast<Sphere*>(Q.object)->r;
    area = r * r * 4 * PI;
  }
  if (dynamic_cast<Mesh*>(Q.object))
  {
    area = reinterpret_cast<Mesh*>(Q.object)->area;
  }
  if (dynamic_cast<Cylinder*>(Q.object))
  {
    Cylinder* cylinder = reinterpret_cast<Cylinder*>(Q.object);

    float r = cylinder->r;
    float len = cylinder->A.norm();

    area = 2.0 * PI * r * len;
    area += 2.0f * PI * r * r;
  }
  if (dynamic_cast<Box*>(Q.object))
  {
    Box* box = reinterpret_cast<Box*>(Q.object);

    area = 2.0f * box->diag.x() * box->diag.y();
    area += 2.0f * box->diag.x() * box->diag.z();
    area += 2.0f * box->diag.z() * box->diag.y();
  }

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
  size_t size = strings.size();
  if (size == 0) return;
  std::string c = strings[0];
  bool dof = false;

  if (strings[size - 1] == "dof")
  {
    dof = true;
  }

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
    raytrace->setCamera(Vector3f(f[1], f[2], f[3]), Orientation(5, strings, f), f[4], f[10]);
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
    if (f.size() < 8)
    {
      currentMat = new Material(Vector3f(f[1], f[2], f[3]), Vector3f(f[4], f[5], f[6]), f[7]);
    }
    else
    {
      currentMat = new Material(Vector3f(f[1], f[2], f[3]), Vector3f(f[4], f[5], f[6]), f[7], Vector3f(f[8], f[9], f[10]), f[11]);
    }
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

    if (dof)
    {
      raytrace->setDOF(Vector3f(f[1], f[2], f[3]));
    }
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

    if (dof)
    {
      raytrace->setDOF((Vector3f(f[1], f[2], f[3]) + Vector3f(f[4], f[5], f[6])) / 2.0f);
    }
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

    if (dof)
    {
      raytrace->setDOF(Vector3f(f[1], f[2], f[3]));
    }
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

    if (dof)
    {
      raytrace->setDOF(Vector3f(f[2], f[3], f[4]));
    }
  }


  else {
    fprintf(stderr, "\n*********************************************\n");
    fprintf(stderr, "* Unknown command: %s\n", c.c_str());
    fprintf(stderr, "*********************************************\n\n");
  }
  }

void Scene::TraceImage(Color* image)
{
#ifdef REAL
  realtime->run();                          // Remove this (realtime stuff)
#endif // REAL

#ifndef REAL
  Vector3f X = raytrace->rx * raytrace->orient._transformVector(Vector3f::UnitX());
  Vector3f Y = raytrace->ry * raytrace->orient._transformVector(Vector3f::UnitY());
  Vector3f Z = -1.0f * raytrace->orient._transformVector(Vector3f::UnitZ());
  float D = raytrace->D;

 #pragma omp parallel for schedule(dynamic, 1) // Magic: Multi-thread y loop
  for (int y = 0; y < height; y++)
  {
    //fprintf(stderr, "Rendering %4d\r", y);

    for (int x = 0; x < width; x++)
    {
      float rand1 = myrandom(RNGen);
      float rand2 = myrandom(RNGen);

      float dx = (2.0f * (x + rand1) / width) - 1.0f;
      float dy = (2.0f * (y + rand2) / height) - 1.0f;

      float r = raytrace->W * sqrt(rand1);
      float theta = 2 * PI * r * rand2;
      float rx = r * cos(theta);
      float ry = r * sin(theta);

      Vector3f pos = raytrace->eye + (rx * X) + (ry * Y);

      Vector3f dir = ((D * dx - rx) * X) + ((D * dy - ry) * Y) + (D * Z);
      dir.normalize();

      Ray ray(pos, dir);
      image[y * width + x] += raytrace->trace(ray, Tree);
    }
  }

  //fprintf(stderr, "\n");
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
  Mesh* mesh = new Mesh(meshdata->mat);

  size_t size = meshdata->triangles.size();

  for (size_t i = 0; i < size; i++)
  {
    TriData& tri = meshdata->triangles[i];
    VertexData v0 = meshdata->vertices[tri[0]];
    VertexData v1 = meshdata->vertices[tri[1]];
    VertexData v2 = meshdata->vertices[tri[2]];

    Triangle* obj = new Triangle(v0.pnt, v1.pnt, v2.pnt, v0.nrm, v1.nrm, v2.nrm, meshdata->mat);
    obj->mesh = mesh;
    mesh->triangles.push_back(obj);

    float a = (v1.pnt - v0.pnt).norm();
    float b = (v2.pnt - v0.pnt).norm();
    float c = (v2.pnt - v1.pnt).norm();

    float s = 0.5f * (a + b + c);
    float area = sqrt(s * (s - a) * (s - b) * (s - c));
    mesh->area += area;

    shapes.push_back(obj);
  }

  if (meshdata->mat->isLight())
  {
    lights.push_back(mesh);
  }
}

RayTrace::RayTrace()
{}

Intersection RayTrace::TraceRay(Ray& ray, KdBVH<float, 3, Shape*>& tree)
{
  Minimizer mini(ray);
  BVMinimize(tree, mini);
  return mini.record;
}

Color RayTrace::trace(Ray& ray, KdBVH<float, 3, Shape*>& tree)
{
  Color C = Color(0.0f, 0.0f, 0.0f);
  Color W = Color(1.0f, 1.0f, 1.0f);

  Intersection P = TraceRay(ray, tree);
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

    Intersection L = SampleLight();
    p = PdfLight(L) / GeometryFactor(P, L);
    Wi = L.P - P.P;
    Wi.normalize();

    float q = PdfBrdf(Wo, Wi, P) * RussianRoulette;
    float Wmis = (p * p) / (p * p + q * q);

    Intersection I = TraceRay(Ray(P.P, Wi), tree);

    if (p > 0.0f && I.object && I.object == L.object)
    {
      Color f = EvalScattering(Wo, P, Wi);

      Color CorruptCheck = W * Wmis * (f / p) * EvalRadiance(L);

      if (!IsCorrupt(CorruptCheck))
      {
        C += CorruptCheck;
      }
    }

    ////////////////////////////////////////////////////////
    N = P.N;
    Wi = SampleBrdf(Wo, P).normalized();

    Intersection Q = TraceRay(Ray(P.P, Wi), tree);

    if (!Q.object)
    {
      break;
    }

    Color f = EvalScattering(Wo, P, Wi);
    p = PdfBrdf(Wo, Wi, P) * RussianRoulette;

    if (p < 0.000001f)
    {
      break;
    }

    Color CorruptCheck = f / p;

    if (IsCorrupt(CorruptCheck))
    {
      break;
    }

    W *= CorruptCheck;

    if (Q.object->mat->isLight())
    {
      q = PdfLight(Q) / GeometryFactor(P, Q);
      Wmis = (p * p) / (p * p + q * q);

      C += W * Wmis * EvalRadiance(Q);
      break;
    }

    P = Q;
    Wo = -Wi;
  }

  return C * 2.0f;
}

bool Sphere::intersect(const Ray& ray, Intersection& intersect)
{
  const Vector3f Q = ray.Q - C;

  const Vector3f D = ray.D;

  const float b = Q.dot(D);

  float disc = (b * b - Q.dot(Q) + r * r);

  if (disc < 0) return false;

  disc = sqrt(disc);

  float t0 = -b + disc;
  float t1 = -b - disc;

  if (t0 < MINPEN && t1 < MINPEN) return false;

  if (t0 < MINPEN)
  {
    intersect.t = t1;
  }
  else if (t1 < MINPEN)
  {
    intersect.t = t0;
  }
  else
  {
    intersect.t = std::min(t0, t1);
  }

  intersect.P = ray.eval(intersect.t);
  intersect.N = (intersect.P - C).normalized();
  intersect.object = this;
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
  intersect.object = mesh;
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