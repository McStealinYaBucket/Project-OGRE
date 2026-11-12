"""Install the matched third-party sources from staging/ into their build locations under Client/ and 3rdparty/.

Run (from anywhere, a few seconds; needs `patch`):
    python3 -P tools/lib/install_libs.py            # all libraries
    python3 -P tools/lib/install_libs.py lua g3d    # a subset (lua raknet g3d boost)

Each destination is deleted and rebuilt from the pristine upstream copy plus the staged patches, applied
in name order with patch -p1, exactly as the matchers in tools/lib/ do, so the CMake build compiles the
same text that matched (tools/lib/{lua,raknet,g3d,boost}_match.py):

  lua     staging/lua-5.1.4-2013/upstream/src + patches 01-15 -> Client/App/Lua-5.1.4/src
          The patches' stand-in headers (FastLog.h from 11, RbxProtectedString.h from 14) are moved to
          Client/App/Lua-5.1.4/standin, so they never shadow Client/Log/include/FastLog.h for App code
          that has Client/App/Lua-5.1.4/src on its include path.
  raknet  staging/raknet/RakNet-4.034/Source + patches 0001-0003 -> Client/Network/raknet/Source
          (only the 26 TUs the 2013 Network library compiled, plus every header; plugins are left out
          because their .cpp files would otherwise be picked up by globs and linked under /OPT:NOREF).
  g3d     "2013 55% App/Rendering/g3d"/{G3Dcpp,include} (read, never edited) + G3D patches
          (staging/g3d-8.00/G3D/patches, 0001-0010 as of 2026-11-12)
          -> Client/Rendering/g3d/{G3Dcpp,include}; zlib 1.2.3 -> Client/Rendering/g3d/zlib;
          libzip 0.9 lib/ + patch -> Client/Rendering/g3d/zip; IJG 6b library sources + patches ->
          Client/Rendering/g3d/ijg (built only to resolve GImage_jpeg's externals; nothing of it is
          linked in PLAYER). No libpng: PLAYER has none; Client/Rendering/g3d/shim (hand-written, not
          installed here) declares png_sig_cmp and stubs GImage_png's two members so the exe links.
  boost   staging/boost_1_53_0 (pristine 1.53.0 library sources + LICENSE) -> 3rdparty/boost_1_53_0
"""
import glob
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ST = os.path.join(ROOT, "staging")
PRIOR_G3D = os.path.join(ROOT, "2013 55% App/Rendering/g3d")
RAKNET_TUS = ("BitStream CCRakNetSlidingWindow CheckSum DS_HuffmanEncodingTree FileOperations GetTime Itoa "
              "LocklessTypes PluginInterface2 RakMemoryOverride RakNetSocket RakNetStatistics RakNetTypes RakPeer "
              "RakSleep RakString RakThread Rand ReliabilityLayer SignaledEvent SimpleMutex SocketLayer "
              "StringCompressor StringTable SuperFastHash WSAStartupSingleton").split()
# makefile.vc LIBOBJECTS of IJG 6b; G3D's bundled copy uses jmemansi (tools/lib/g3d_match.py JPEG).
JPEG = ("jcapimin jcapistd jctrans jcparam jdatadst jcinit jcmaster jcmarker jcmainct jcprepct jccoefct "
        "jccolor jcsample jchuff jcphuff jcdctmgr jfdctfst jfdctflt jfdctint jdapimin jdapistd jdtrans "
        "jdatasrc jdmaster jdinput jdmarker jdhuff jdphuff jdmainct jdcoefct jdpostct jddctmgr jidctfst "
        "jidctflt jidctint jidctred jdsample jdcolor jquant1 jquant2 jdmerge jcomapi jutils jerror jmemmgr "
        "jmemansi").split()


def fresh(dst, src, keep=lambda name: True):
    shutil.rmtree(dst, ignore_errors=True)
    shutil.copytree(src, dst, ignore=lambda d, names: [n for n in names
                                                         if os.path.isfile(os.path.join(d, n)) and not keep(n)])


def patch(dst, patches):
    for p in sorted(patches):
        r = subprocess.run(["patch", "-p1", "-s", "--no-backup-if-mismatch", "-d", dst, "-i", p],
                           capture_output=True, text=True)
        if r.returncode:
            sys.exit(f"{p}: {r.stdout}{r.stderr}")


def lua():
    src = os.path.join(ROOT, "Client/App/Lua-5.1.4/src")
    standin = os.path.join(ROOT, "Client/App/Lua-5.1.4/standin")
    fresh(src, os.path.join(ST, "lua-5.1.4-2013/upstream/src"), lambda n: n != "Makefile")
    patch(src, glob.glob(os.path.join(ST, "lua-5.1.4-2013/patches/*.patch")))
    shutil.rmtree(standin, ignore_errors=True)
    os.makedirs(standin)
    for h in ("FastLog.h", "RbxProtectedString.h"):
        shutil.move(os.path.join(src, h), os.path.join(standin, h))


def raknet():
    dst = os.path.join(ROOT, "Client/Network/raknet/Source")
    shutil.rmtree(os.path.dirname(dst), ignore_errors=True)
    fresh(dst, os.path.join(ST, "raknet/RakNet-4.034/Source"),
          lambda n: n.endswith(".h") or n[:-4] in RAKNET_TUS and n.endswith(".cpp"))
    shutil.rmtree(os.path.join(dst, "cat"), ignore_errors=True)  # LIBCAT_SECURITY is compiled out
    patch(dst, glob.glob(os.path.join(ST, "raknet/patches/*.patch")))


def g3d():
    base = os.path.join(ROOT, "Client/Rendering/g3d")
    for d in ("G3Dcpp", "include"):
        fresh(os.path.join(base, d), os.path.join(PRIOR_G3D, d))
    patch(base, glob.glob(os.path.join(ST, "g3d-8.00/G3D/patches/*.patch")))
    fresh(os.path.join(base, "zlib"), os.path.join(ST, "g3d-8.00/zlib-1.2.3/upstream"))
    tmp = os.path.join(base, "_zip")  # the libzip patch is rooted at lib/
    fresh(os.path.join(tmp, "lib"), os.path.join(ST, "g3d-8.00/libzip-0.9/upstream/lib"),
          lambda n: n.endswith((".c", ".h")))
    patch(tmp, glob.glob(os.path.join(ST, "g3d-8.00/libzip-0.9/patches/*.patch")))
    shutil.rmtree(os.path.join(base, "zip"), ignore_errors=True)
    os.rename(os.path.join(tmp, "lib"), os.path.join(base, "zip"))
    os.rmdir(tmp)
    fresh(os.path.join(base, "ijg"), os.path.join(ST, "g3d-8.00/jpeg-6b/upstream"),
          lambda n: n.endswith(".h") or n.endswith(".vc") or n.endswith(".c") and n[:-2] in JPEG)
    patch(os.path.join(base, "ijg"), glob.glob(os.path.join(ST, "g3d-8.00/jpeg-6b/patches/*.patch")))


def boost():
    fresh(os.path.join(ROOT, "3rdparty/boost_1_53_0"), os.path.join(ST, "boost_1_53_0"))


if __name__ == "__main__":
    steps = {"lua": lua, "raknet": raknet, "g3d": g3d, "boost": boost}
    for name in sys.argv[1:] or steps:
        steps[name]()
        print("installed", name)
