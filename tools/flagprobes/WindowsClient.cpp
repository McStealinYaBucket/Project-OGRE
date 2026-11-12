// probe: WindowsClient (PLAYER client shell 0x401000-0x46c000, which also compiles Client/Win and
// Client/ClientBase). No iOS/Mac symbols exist for Windows-only code, so the probed functions are unnamed;
// they were chosen for their x87-only float code (fnstsw compares, no SSE anywhere in this range).
// 0x00429ce0: double -> unsigned clamp. 0x0040bfa0: unsigned -> float scale, then a clamped float -> unsigned.
unsigned int FUN_00429ce0(double d)
{
  if (d >= 4294967295.0)
    return 0xffffffffu;
  return (unsigned int)d;
}
class C0040bfa0 { public: int pad; unsigned int count; int pad2[3]; float scale; unsigned int result; void FUN_0040bfa0(); };
void C0040bfa0::FUN_0040bfa0()
{
  float v = (float)count * scale;
  result = 0xffffffffu;
  if (4294967296.0 > v)
    result = (unsigned int)v;
}

// RBX::Name::doDeclare<sScript> (0x00413740), instantiated in a client-shell TU: the EH frame carries the
// __security_cookie (/GS). Same source as the RbxOgre probe's doDeclare<sDecal> (0x00480bf0), which has none.
namespace RBX {
class Name { public:
  static const Name& declare(const char* const& sName);
  template<const char* const& sName> static const Name& doDeclare() { static const Name* n = &declare(sName); return *n; }
};
extern const char* const sScript;
template const Name& Name::doDeclare<sScript>();
}
