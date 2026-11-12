# lib-g3d (workflow phase2-skeleton, wf_e3afb0d0-6b4)

G3D 8.00 and its bundled libraries in PLAYER, measured.

**Sources.** I could not get a pristine G3D 8.00 archive from upstream. Sourceforge deleted g3d-cpp/8.00 between 2024-03 (still in the Wayback listing) and now, and the g3d SVN starts at G3D 9. So the G3D base is the existing tree, "2013 55% App/Rendering/g3d". I read it in place and staged only patches. Its .cpp files are byte-identical to the 2016 Roblox fork. Its headers differ from 2016 only in Color3.h (black/white bodies moved into the class), a comment in Color4.h, and 0-byte stubs for AtomicInt32.h and ReferenceCount.h. That tree is not what PLAYER has:
- It has G3D_VER 80000, but carries IJG 7 ("27-Jun-2009"), libpng 1.2.40 and an empty zip/ directory, and expects an external zlib 1.2.8. It also has 2016 fast flags and Bullet.
- PLAYER has zlib 1.2.3, libzip 0.9 and IJG 6b, and no libpng.
- "2013 55% App/G3D/include" is a 23-header shadow subset of the same fork, in no include path.

The other three come from official upstream, checksums verified where published: zlib 1.2.3 (zlib.net/fossils, sha256 1795c7d0… matches), jpeg-6b (ijg.org, 75c3ec24…) and libzip 0.9 (libzip.org, b0d0a768…, no published hash). libzip 0.9 is pinned rather than 0.9.1/0.9.3: zip_fread at 0x009e8460 is identical with 0.9, and 0.9.1 rewrote that function. G3D's MSVC edits to its bundled copies come from G3D's own SVN (r100 zip.lib/jpeg.lib). Upstream G3D 8.x text for the patches comes from SVN r30.

**Flags.** The App flags are re-proven on this library: /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast, /Gy, C files as C, /DZLIB_WINAPI (see the flags field).

**PLAYER results.** zlib 45/45 identical, libzip 65/65 identical. G3D: 161 identical rows (158 non-inline rows at 132 distinct addresses, because /OPT:ICF folds identical bodies like Vector3::xy and Vector4::xy). 22 G3D functions were located but are not identical. 133 functions inside the block have no compiled counterpart. IJG 6b: 0 functions linked. Only jerror.obj's message table survives, at 0x00bc2250; nothing references it, and D3DX's copy is at 0x00bd2150. /Gy plus /OPT:REF dropped every jpeg function, including GImage_jpeg.

**Block and link order (PLAYER).** RBX::AdornBillboarder ends at 0x009d5b70, then:
- G3D::Log 0x009d5b70, debugAssert 0x009d6080, Vector2, Vector2int16, System 0x009d6cf0, Vector3, Color3 0x009d7a40, g3dmath, Matrix3 0x009d7f30, CoordinateFrame 0x009d9550, Vector3/Vector4/Matrix4, MemoryManager, Color3uint8/Color4uint8/Vector3int16.
- BinaryInput 0x009db5c0, fileutils, FileSystem (Table/Set instances, 0x009dc240–0x009ddb90; not in the fork), Random 0x009de8d0, GImage 0x009deea0, CollisionDetection 0x009df4e0.
- zlib crc32 0x009e0420, deflate 0x009e0720–0x009e1e80, inflate 0x009e20a0–0x009e37d0.
- G3D Color4, Plane, LightingParameters 0x009e3b70, Sphere, format, Line, Quat, Cylinder, prompt 0x009e56b0, Crypto, stringutils, fileutils, Ray, Box.
- zlib uncompress 0x009e7260; libzip zip_close…zip_open 0x009e7310–0x009e9010; G3D g3dfnmatch; libzip zip_stat_index, zip_get_num_files; G3D LineSegment.
- zlib adler32, trees, zutil, inffast 0x009eb670, inftrees 0x009ebad0; libzip zip_name_locate … zip_unchange_data 0x009edb40.
- Four unattributed functions at 0x009edb80–0x009edf00, then RBX::Frustum (RbxG3D) at 0x009ee010.

zlib, libzip and G3D objects interleave in resolution order, so they were most likely one static library, as in the 2016 graphics3D project. STUDIO's main block is 0x006aa500–0x006c2070, with LightingParameters elsewhere (around 0x00a97c70).

**STUDIO results.** zlib 45, libzip 65, G3D 162 identical; 22 not identical; 157 unclaimed.

**Roblox edits, written as minimal patches from PLAYER bytes** (staging/g3d-8.00/G3D/patches):
- 0001 Color3 black()/white() out of line again (white is at 0x009d7c70).
- 0002 Vector3 regains upstream's user-defined `Vector3& __fastcall operator=`. Proof: Plane::getEquation at 0x009e3b40 copies with fld/fstp. This made 8 more functions identical. **It also matters for all App code, whose Vector3.h lacks this operator** (I did not edit CLAUDE.md).
- 0003 Random goes back to upstream's thread-safe version: Spinlock at +4, m_threadsafe at +0x10, ctor `ret 8`; ctor, bits and generate now identical.
- 0004 Matrix3::toAxisAngle without DFFlag::FixMatrixToAxisAngle (0x009d8910 now identical).
- 0005 Quat(Matrix3) without DFFlag::G3DQuatConstructorFix (0x009e5020 now identical).

Patches for zlib/jpeg/libzip are build needs only: jconfig.h from jconfig.vc, G3D's jmorecfg INT32 guard, and G3D's MSVC port of libzip.

**Remaining differences.** None of these is a flag effect, since every flag flip loses functions:
- System is 0x20 bytes bigger in 2013 (getStandardProcessorExtensions compares at +0xc8 where ours uses +0xa8).
- GImage::Format has two more enum values (stringToFormat returns 9 where ours returns 7).
- The 2016 fork stubs out BinaryInput::decompress and loadIntoMemory.
- Bodies differ in CollisionDetection::rayAABox and findRayCapsuleIntersection[Aux], LightingParameters::setTime, Ray::set, Box::init, CoordinateFrame::lookAt and lerp, Quat::toRotationMatrix, and Vector3::ignore.
- 0x009d8000 is the folded Matrix3::operator==; the Matrix4 and GLight operator== rows on it are mislocations.
- The ~Random and ~BinaryInput rows landed on scalar deleting destructors.

The matcher imports tools/lib/lua_match.py, which another agent kept editing during my run.

## Structured fields

```json
{
 "files": [
  "tools/lib/g3d_match.py",
  "ida/exports/PLAYER-lib-g3d.csv",
  "staging/g3d-8.00/zlib-1.2.3/upstream",
  "staging/g3d-8.00/jpeg-6b/upstream",
  "staging/g3d-8.00/jpeg-6b/patches/0001-jconfig-h-from-jconfig-vc.patch",
  "staging/g3d-8.00/jpeg-6b/patches/0002-g3d-jmorecfg-basetsd-guard.patch",
  "staging/g3d-8.00/libzip-0.9/upstream",
  "staging/g3d-8.00/libzip-0.9/patches/0001-g3d-msvc-port.patch",
  "staging/g3d-8.00/G3D/patches/0001-color3-out-of-line-black-white.patch",
  "staging/g3d-8.00/G3D/patches/0002-vector3-user-operator-assign.patch",
  "staging/g3d-8.00/G3D/patches/0003-random-upstream-spinlock-threadsafe.patch",
  "staging/g3d-8.00/G3D/patches/0004-matrix3-toaxisangle-no-fastflag.patch",
  "staging/g3d-8.00/G3D/patches/0005-quat-ctor-no-fastflag.patch",
  "3rdparty/_downloads/jpeg-6b",
  "3rdparty/_downloads/libzip-0.9",
  "3rdparty/_downloads/libzip-0.9.1",
  "3rdparty/_downloads/libzip-0.9.3",
  "3rdparty/_downloads/g3d-svn-r100",
  "3rdparty/_downloads/g3d-svn-r30"
 ],
 "csv": "ida/exports/PLAYER-lib-g3d.csv",
 "matched_identical": 543,
 "located_not_identical": 44,
 "flags": "All G3D, zlib and libzip objects: /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /MD /GR /EHsc /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB, plus /Gy (function-level linking).\n- zlib, libzip and IJG are compiled as C (/TC) with /DZLIB_WINAPI; G3D as C++ (/TP).\n- Each single-flag flip measured against this set on PLAYER, with the identical functions it loses: /Oy- 233, /arch:SSE2 65, no /arch (x87) 79, /fp:precise 53, /GS 21. libzip compiled as C++ matched 55 instead of 66.\n\nDeciding functions:\n- /Oy: _zcfree 0x009eb660 and _zcalloc 0x009eb640.\n- /arch:SSE, not SSE2: _crc32@12 0x009e0700 and _gen_codes 0x009eae50.\n- /arch:SSE, not x87: _flush_pending 0x009e0750 and _updatewindow 0x009e2200.\n- /fp:fast: G3D::fnan 0x009d7e50 and G3D::Random::hemi 0x009ded20.\n- /GS-: _gen_codes 0x009eae50 and _inflate_table 0x009ebad0.\n- ZLIB_WINAPI (__stdcall): _deflate@8 at 0x009e07a0 ends in `ret 8`.\n- /Gy: deflateInit2_ is linked at 0x009e1e80, but deflateInit_, deflateParams, deflateCopy and every IJG function are absent. jerror.obj's message table is still present (0x00bc2250) with no code referencing it.\n\nNot tested: /Ob1, /Oi-. The 2016 graphics3D.vcxproj values /Oy-, /arch:SSE2 and no /Gy are all wrong for 2013.",
 "unresolved": [
  "Pristine G3D 8.00 source could not be obtained from upstream: sourceforge g3d-cpp/8.00 (G3D-8.00-src.zip) is deleted and the g3d SVN starts at G3D 9. G3D was compiled from the existing tree's 2016 Roblox fork plus 5 patches; upstream G3D 8.x text came from g3d SVN r30 (G3D 9.00.01) where needed.",
  "22 G3D functions are located but not identical in PLAYER (same count in STUDIO): G3D::System initTime, getStandardProcessorExtensions, init and System() (2013 class is 0x20 bytes larger); BinaryInput::decompress and loadIntoMemory (stubs in the 2016 fork); GImage::resolveFormat (Format enum has 2 more values in 2013); CollisionDetection::rayAABox, findRayCapsuleIntersection[Aux]; LightingParameters::setTime; Ray::set; Box::init; CoordinateFrame::lookAt and lerp; Quat::toRotationMatrix; Vector3::ignore. Not patched.",
  "Rows that are mislocations: 0x009d8000 is the ICF-folded Matrix3::operator==, so the Matrix4 and GLight operator== rows on it are wrong; the G3D::Random::~Random and BinaryInput::~BinaryInput rows landed on scalar deleting destructors (0x009dee00, 0x009db7a0).",
  "133 PLAYER functions inside the G3D block have no compiled counterpart, mostly G3D files absent from the 2016 fork: Log (0x009d5b70), debugAssert's createErrorMessage and _handleErrorCheck_ (0x009d6080, 0x009d6780), FileSystem with its Table and Set instances (0x009dc240-0x009ddb90), prompt (0x009e56b0), BinaryInput::BinaryInput (0x009dbc50) and others.",
  "Four functions at 0x009edb80-0x009edf00, between libzip's last function and RBX::Frustum (0x009ee010), are not attributed to any library.",
  "IJG 6b contributes no linked functions to PLAYER or STUDIO, only data (jerror message table 0x00bc2250); GImage_jpeg is not linked. G3D's jmemansi changes were therefore not staged.",
  "STUDIO puts LightingParameters outside its main block (around 0x00a97c70). setTime there is not identical and was dropped by the outside-block filter, so STUDIO's link order around it is not mapped.",
  "tools/lib/g3d_match.py imports the locating engine from tools/lib/lua_match.py, which another agent edited during this run (the locate() signature changed once). Re-running after further edits there can change the CSV.",
  "Patch 0002 (the user-defined Vector3::operator= __fastcall) implies the existing App tree's Vector3.h is wrong for 2013 codegen. CLAUDE.md Codegen Patterns and the App headers are not updated (outside this task)."
 ]
}
```

