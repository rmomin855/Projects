///////////////////////////////////////////////////////////////////////
// A framework for a raytracer.
////////////////////////////////////////////////////////////////////////
#ifndef RAY
#define RAY

#include <algorithm>

class Shape;

const float PI = 3.14159f;
const float Radians = PI / 180.0f;    // Convert degrees to radians

////////////////////////////////////////////////////////////////////////
// Material: encapsulates a BRDF and communication with a shader.
////////////////////////////////////////////////////////////////////////
class Material
{
public:
  Vector3f Kd, Ks, Kt;
  float alpha, IOR;
  unsigned int texid;

  float pd;
  float pr;
  float pt;
  bool light;

  bool isLight() { return light; }

  Material() : Kd(Vector3f(1.0, 0.5, 0.0)), Ks(Vector3f(1, 1, 1)), alpha(1.0), texid(0), Kt(), IOR(1.0f)
  {
    float S = Kd.norm() + Ks.norm() + Kt.norm();
    pd = Kd.norm() / S;
    pr = Ks.norm() / S;
    pt = Kt.norm() / S;
    light = false;
  }
  Material(const Vector3f d, const Vector3f s, const float a)
    : Kd(d), Ks(s), alpha(a), texid(0), Kt(), IOR(1.0f)
  {
    float S = Kd.norm() + Ks.norm() + Kt.norm();
    pd = Kd.norm() / S;
    pr = Ks.norm() / S;
    pt = Kt.norm() / S;
    light = false;
  }

  Material(Material& o)
  {
    Kd = o.Kd;  Ks = o.Ks;  Kt = o.Kt; IOR = o.IOR; alpha = o.alpha;  texid = o.texid;
    float S = Kd.norm() + Ks.norm() + Kt.norm();
    pd = Kd.norm() / S;
    pr = Ks.norm() / S;
    pt = Kt.norm() / S;
    light = o.light;
  }
  Material(const Vector3f d, const Vector3f s, const float a, const Vector3f t, const float Ior)
    : Kd(d), Ks(s), alpha(a), texid(0), Kt(t), IOR(Ior)
  {
    float S = Kd.norm() + Ks.norm() + Kt.norm();
    pd = Kd.norm() / S;
    pr = Ks.norm() / S;
    pt = Kt.norm() / S;
    light = false;
  }

  void setTexture(const std::string path);
  //virtual void apply(const unsigned int program);
};

////////////////////////////////////////////////////////////////////////
// Data structures for storing meshes -- mostly used for model files
// read in via ASSIMP.
//
// A MeshData holds two lists (stl::vector) one for vertices
// (VertexData: consisting of point, normal, texture, and tangent
// vectors), and one for triangles (TriData: consisting of three
// indices into the vertex array).
typedef Eigen::Matrix<unsigned int, 3, 1 > TriData;

class VertexData
{
public:
  Vector3f pnt;
  Vector3f nrm;
  Vector2f tex;
  Vector3f tan;
  VertexData(const Vector3f& p, const Vector3f& n, const Vector2f& t, const Vector3f& a)
    : pnt(p), nrm(n), tex(t), tan(a)
  {}
};

struct MeshData
{
  std::vector<VertexData> vertices;
  std::vector<TriData> triangles;
  Material* mat;
};

////////////////////////////////////////////////////////////////////////
// Obj: encapsulates objects to be drawn; uses OpenGL's VAOs
////////////////////////////////////////////////////////////////////////
class Obj
{
public:
  MeshData* meshdata;
  Matrix4f modelTR;
  Material* material;
  Vector3f center;
  float area;
  unsigned int vao;
  Obj() = default;
  Obj(MeshData* m, const Matrix4f& tr, Material* b);
  void draw();
  Vector3f Center() { return center; }
};

////////////////////////////////////////////////////////////////////////
// Light: encapsulates a light and communiction with a shader.
////////////////////////////////////////////////////////////////////////
class Light : public Material
{
public:

  Light(const Vector3f e) : Material() { Kd = e; light = true; }
  //virtual void apply(const unsigned int program);
};

////////////////////////////////////////////////////////////////////////////////
// Scene
class Realtime;

class Ray
{
public:
  Vector3f Q;//starting Point
  Vector3f D;//direction

  Ray(Vector3f pos, Vector3f dir) : Q(pos), D(dir) {}

  Vector3f eval(float t) const { return Q + (t * D); }
};

class Intersection
{
public:
  Intersection() = default;
  float t = FLT_MAX; //distance along the ray
  Shape* object = nullptr; //object interacted with
  Vector3f P; //point of intersection
  Vector3f N; //normal of intersection
};

class Slab
{
public:
  Slab() = default;
  Slab(Vector3f N, float D0, float D1) :n(N), d0(D0), d1(D1) {}

  Vector3f n;
  float d0;
  float d1;
};

class Interval
{
public:
  float t0, t1;
  Vector3f n0, n1;

  Interval() : t0(0.0f), t1(FLT_MAX) {}
  Interval(float T0, float T1, Vector3f N0 = Vector3f(), Vector3f N1 = Vector3f()) :
    t0(T0), t1(T1), n0(N0), n1(N1) {}

  void empty() { t0 = 0.0f; t1 = -1.0f; }
  void intersect(Interval& other);
  static Interval intersectSlab(const Ray& ray, Slab& slab);
};

class Shape
{
public:
  Material* mat;
  Shape() = default;
  virtual bool intersect(const Ray& ray, Intersection& intersect) { return false; }
  virtual Bbox bbox() const { return Bbox(); }
};

class Box : public Shape
{
public:
  Vector3f base, diag;
  Slab slabs[3] = {};

  Box(Vector3f Base, Vector3f Diag, Material* m) :
    base(Base), diag(Diag)
  {
    mat = m;

    for (size_t i = 0; i < 3; i++)
    {
      slabs[i].n = Vector3f::Zero();
      slabs[i].n[i] = 1.0f;
      slabs[i].d0 = -base[i];
      slabs[i].d1 = -base[i] - diag[i];
    }
  }
  bool intersect(const Ray& ray, Intersection& intersect);

  Bbox bbox() const
  {
    return Bbox(base, base + diag);
  }
};

class Sphere : public Shape
{
public:
  Vector3f C;//center
  float r;//radius

  Sphere(const Vector3f center, const float R, Material* m) : C(center), r(R)
  {
    mat = m;
  }

  bool intersect(const Ray& ray, Intersection& intersect);

  Bbox bbox() const
  {
    return Bbox(C - Vector3f(r, r, r), C + Vector3f(r, r, r));
  }
};

class Cylinder : public Shape
{
public:
  Vector3f B;//base
  Vector3f A;//axis
  float r;//radius

  Cylinder(const Vector3f Base, const Vector3f Axis, const float Radius, Material* m)
    : B(Base), A(Axis), r(Radius)
  {
    mat = m;
  }

  bool intersect(const Ray& ray, Intersection& intersect);

  Bbox bbox() const
  {
    Vector3f P0 = B + Vector3f(r, r, r);
    Vector3f P1 = B - Vector3f(r, r, r);
    Vector3f P2 = A + B + Vector3f(r, r, r);
    Vector3f P3 = A + B - Vector3f(r, r, r);

    Vector3f minPoint;
    minPoint[0] = std::fmin(std::fmin(P0[0], P1[0]), std::fmin(P2[0], P3[0]));
    minPoint[1] = std::fmin(std::fmin(P0[1], P1[1]), std::fmin(P2[1], P3[1]));
    minPoint[2] = std::fmin(std::fmin(P0[2], P1[2]), std::fmin(P2[2], P3[2]));

    Vector3f maxPoint;
    maxPoint[0] = std::fmax(std::fmax(P0[0], P1[0]), std::fmax(P2[0], P3[0]));
    maxPoint[1] = std::fmax(std::fmax(P0[1], P1[1]), std::fmax(P2[1], P3[1]));
    maxPoint[2] = std::fmax(std::fmax(P0[2], P1[2]), std::fmax(P2[2], P3[2]));

    return Bbox(minPoint, maxPoint);
  }
};

class Mesh;

class Triangle : public Shape
{
public:
  Vector3f V0, V1, V2, N0, N1, N2;
  Mesh* mesh;

  Triangle(Vector3f v0, Vector3f v1, Vector3f v2, Vector3f n0, Vector3f n1, Vector3f n2, Material* m)
    : V0(v0), V1(v1), V2(v2), N0(n0), N1(n1), N2(n2)
  {
    mat = m;
  }

  bool intersect(const Ray& ray, Intersection& intersect);

  Bbox bbox() const
  {
    Vector3f minPoint;
    minPoint[0] = std::fmin(std::fmin(V0[0], V1[0]), V2[0]);
    minPoint[1] = std::fmin(std::fmin(V0[1], V1[1]), V2[1]);
    minPoint[2] = std::fmin(std::fmin(V0[2], V1[2]), V2[2]);

    Vector3f maxPoint;
    maxPoint[0] = std::fmax(std::fmax(V0[0], V1[0]), V2[0]);
    maxPoint[1] = std::fmax(std::fmax(V0[1], V1[1]), V2[1]);
    maxPoint[2] = std::fmax(std::fmax(V0[2], V1[2]), V2[2]);

    return Bbox(minPoint, maxPoint);
  }
};

class Mesh : public Shape
{
public:

  Mesh(Material* m) : triangles(), area()
  {
    mat = m;
  }

  std::vector<Triangle*> triangles;
  float area;
};

class RayTrace
{
public:
  Color ambient;
  Vector3f eye;      // Position of eye for viewing scene
  Quaternionf orient;   // Represents rotation of -Z to view direction
  float ry;
  float rx;
  float D, W;

  int width, height;

  std::vector<Shape*> shapes;
  std::vector<Shape*> lights;

  void setScreen(const int _width, const int _height) { width = _width;  height = _height; }
  void setCamera(const Vector3f& _eye, const Quaternionf& _o, const float _ry, float w)
  {
    eye = _eye; orient = _o; ry = _ry;
    rx = ry * width / height;
    W = w;
    D = 10.0f;
  }
  void setAmbient(const Vector3f& _a) { ambient = _a; }
  void setDOF(const Vector3f& pos)
  {
    D = (pos - eye).norm();
  }

  void sphere(const Vector3f center, const float r, Material* mat);
  void box(const Vector3f base, const Vector3f diag, Material* mat);
  void cylinder(const Vector3f base, const Vector3f axis, const float radius, Material* mat);
  void triangleMesh(MeshData* meshdata);

  RayTrace();
  Color trace(Ray& ray, KdBVH<float, 3, Shape*>& tree);
  Intersection TraceRay(Ray& ray, KdBVH<float, 3, Shape*>& tree);

  Intersection SampleLight();
  float PdfLight(Intersection& Q);
  Color EvalRadiance(Intersection& Q);
};

class Scene {
public:
  int width, height;
  Realtime* realtime;         // Remove this (realtime stuff)
  RayTrace* raytrace;

  Material* currentMat;
  KdBVH<float, 3, Shape*> Tree;

  Scene();
  void Finit();

  // The scene reader-parser will call the Command method with the
  // contents of each line in the scene file.
  void Command(const std::vector<std::string>& strings,
    const std::vector<float>& f);

  // To read a model file into the scene via ASSIMP, call ReadAssimpFile.  
  void ReadAssimpFile(const std::string& path, const Matrix4f& M);

  // Once ReadAssimpFile parses the information from the model file,
  // it will call:
  void triangleMesh(MeshData* mesh);

  // The main program will call the TraceImage method to generate
  // and return the image.  This is the Ray Tracer!
  void TraceImage(Color* image);
};

class Minimizer
{
public:
  typedef float Scalar; // KdBVH needs Minimizer::Scalar defined
  Ray ray;
  Intersection record;
  Shape* shape;

  // Stuff to track the minimal t and its intersection info
  // Constructor
  Minimizer(const Ray& r) : ray(r), record(), shape(nullptr)
  {}

  void Reset(Vector3f Q, Vector3f D)
  {
    ray.Q = Q;
    ray.D = D;
    record.object = nullptr;
  }

  void Reset(Ray Ray)
  {
    ray = Ray;
    record.object = nullptr;
  }
  // Called by BVMinimize to intersect the ray with a Shape. This
  // should return the intersection t, but should also track
  // the minimum t and it's corresponding intersection info.
  // Return INF to indicate no intersection.
  float minimumOnObject(Shape* obj);
  // Called by BVMinimize to intersect the ray with a Bbox and
  // returns the t value. This should be similar to the already
  // written box (3 slab) intersection. (The difference being: a
  // distance of zero should be returned if the ray starts within the Bbox.)
  // Return INF to indicate no intersection.
  float minimumOnVolume(const Bbox& box);
};

#endif