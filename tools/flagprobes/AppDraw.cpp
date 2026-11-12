// probe: AppDraw (PLAYER 0x9c1000-0x9c5e10, DrawAdorn.cpp). CircleRadialNormal is file-local in DrawAdorn.cpp;
// its virtuals are emitted out of line. Body after the 2016 source, checked against the 2013 bytes.
#include <G3D/Vector3.h>
#include <math.h>
#include <string>
using G3D::Vector3;
namespace RBX {
enum NormalId { NORM_X = 0 };
class I3DLinearFunc { public:
  virtual Vector3 eval(float t) = 0; virtual Vector3 evalTangent(float t) = 0;
  virtual Vector3 evalNormal(float t) = 0; virtual Vector3 evalBinormal(float t) = 0;
  virtual std::string hashString() = 0; };
class CircleRadialNormal : public I3DLinearFunc
{
  Vector3 x; Vector3 y; Vector3 z; float r; NormalId axis;
public:
  CircleRadialNormal(float radius, NormalId axis);
  Vector3 eval(float t);
  Vector3 evalTangent(float t);
  Vector3 evalNormal(float t);
  Vector3 evalBinormal(float t);
  std::string hashString();
};
Vector3 CircleRadialNormal::eval(float t)          // 0x9c2900
{
  double theta = t * 2 * G3D::pi();
  return x * (float)(cos(theta) * r) + y * (float)(sin(theta) * r);
}
Vector3 CircleRadialNormal::evalTangent(float t)   // 0x9c2990
{
  double theta = t * 2 * G3D::pi();
  return y * (float)cos(theta) - x * (float)sin(theta);
}
Vector3 CircleRadialNormal::evalNormal(float t)    // 0x9c2a10
{
  double theta = t * 2 * G3D::pi();
  return x * (float)cos(theta) + y * (float)sin(theta);
}
Vector3 CircleRadialNormal::evalBinormal(float t)  // 0x9c2a90
{
  return -z;
}
}
