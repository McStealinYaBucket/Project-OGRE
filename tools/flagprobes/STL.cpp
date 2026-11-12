// probe: STL configuration (Phase 3 step 3), compiled with the App flags.
// RBX::Poly::getNumSurfaces() const (PLAYER 0x00754610) is mesh->numFaces(), i.e. faces.size() on a
// std::vector of 40-byte faces that sits at +0x20 of the mesh. The target materialises &faces (add eax,0x20)
// and reads _Mylast/_Myfirst at +0x28/+0x24, i.e. vector+8/+4: the _SECURE_SCL=0 layout (allocator byte, then
// three pointers, 16 bytes). Under _SECURE_SCL=1 VC9 puts _Myownedaux first and the pointers at +0xc/+0x10
// (24 bytes), and the probe stops matching.
#include <string>
#include <vector>
#define DECOMP_SIZE_ASSERT(T, S) typedef char T##_size_assert[sizeof(T) == (S) ? 1 : -1];
namespace RBX {
struct Face { char data[40]; };
typedef std::vector<Face> FaceVector;
typedef std::vector<int>::iterator VectorIterator;
#if !_SECURE_SCL                            // the flip to _SECURE_SCL=1 gives 0x18 and 0x8 (checked)
DECOMP_SIZE_ASSERT(FaceVector, 0x10)        // allocator (empty, padded) + _Myfirst/_Mylast/_Myend
DECOMP_SIZE_ASSERT(VectorIterator, 0x4)     // unchecked iterator: a bare pointer
#else
DECOMP_SIZE_ASSERT(FaceVector, 0x18)
DECOMP_SIZE_ASSERT(VectorIterator, 0x8)
#endif
typedef std::string String;
DECOMP_SIZE_ASSERT(String, 0x1c)            // allocator + _Bx[16] + _Mysize + _Myres (same under both settings)
class Mesh { public: char pad[0x20]; FaceVector faces; int numFaces() const { return (int)faces.size(); } };
class Poly { public: char pad[0x14]; Mesh* mesh; int getNumSurfaces() const; };
int Poly::getNumSurfaces() const { return mesh->numFaces(); }
}
