"""Byte-match probe: compile the existing tree's Lua 5.1.4 with VC9 SP1 and compare against PLAYER.

Run:
    python3 -P tools/msvc9/lua_probe.py [--flags "/O2 ..."] [--no-xmmintrin] [--keep-wrappers]
                                        [--keep-ckey] [-v]

Copies "2013 55% App/App/Lua-5.1.4/src" into a scratch dir (mktemp under /tmp/claude-1000), applies
the probe edits below, compiles six TUs with tools/msvc9/cl, and runs tools/probe_compare.py on the
50 PLAYER functions identified for the 2026-11-12 probe. Prints IDENTICAL/DIFFERENT per function
and a total; -v prints the masked diffs. Nothing is written inside the repo.

Probe edits (scratch only; each is backed by PLAYER bytes, see the run report):
  - luaconf.h: the RBX includes become a stub (rbxstub.h) holding LuaVM.h's shuffle/LuaVMValue
    macros, a 40-byte RobloxExtraSpace and no lua_vmhooked handler; RBX_STUDIO_BUILD is defined so
    LUAVM_SECURE, LuaSecureDouble, the lua_unlock text-address check and the string-hash check are
    off (PLAYER's lua_settop/lua_xmove have no hook, its TValue/Table reads are plain).
  - LuaVMValue<T> fields in lobject.h/lstate.h become plain T (2016 wrapper; luaK_code and luaH_next
    only match without it).                                          (--keep-wrappers to skip)
  - global_State loses the 2016 'ckey' field (lua_atpanic reads panic at G+0x58).  (--keep-ckey)
  - rbxstub.h includes <xmmintrin.h>, standing in for the RBX headers the real luaconf.h pulls in.
    Without it VC9 /arch:SSE passes static-function args in ebx and newlstr/fixjump/patchlistaux
    differ.                                                          (--no-xmmintrin to skip)
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SRC = os.path.join(ROOT, "2013 55% App/App/Lua-5.1.4/src")
LUAVM_H = os.path.join(ROOT, "2013 55% App/App/include/script/LuaVM.h")
FLAGS = "/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast"
COMMON = ("/nologo /c /TP /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB /D_RELEASE "
          "/DRBX_STUDIO_BUILD /D_CRT_SECURE_NO_WARNINGS /W0").split()
PAIRS = {
    "lobject": "luaO_int2fb@0x8a78c0 luaO_fb2int@0x8a78f0 luaO_log2@0x8a7910 luaO_rawequalObj@0x8a7940 "
               "luaO_str2d@0x8a79a0 pushstr@0x8a7a50",
    "lapi": "index2adr@0x873950 lua_xmove@0x873a70 lua_setlevel@0x873ac0 lua_atpanic@0x873ae0 "
            "lua_gettop@0x873b00 lua_settop@0x873b10 lua_type@0x873cf0 lua_typename@0x873d10 "
            "lua_rawequal@0x873dd0 lua_pushnil@0x8740a0 lua_replace@0x873c00 lua_tonumber@0x873e50 "
            "lua_tointeger@0x873e90 lua_pushnumber@0x8740c0 lua_pushinteger@0x8740e0",
    "ltable": "hashnum@0x98be80 findindex@0x98bf50 luaH_next@0x98c010 luaH_getnum@0x98c480 "
              "luaH_getstr@0x98c520 luaH_get@0x98c560 luaH_set@0x98c610 mainposition@0x98bec0 "
              "computesizes@0x98c0f0 setnodevector@0x98c300 unbound_search@0x98c6d0 luaH_getn@0x98c770",
    "lstring": "luaS_resize@0x98cc70 newlstr@0x98cd30 luaS_newlstr@0x98cdd0",
    "lzio": "luaZ_fill@0x98d320 luaZ_lookahead@0x98d360 luaZ_init@0x98d3c0 luaZ_read@0x98d3f0 "
            "luaZ_openspace@0x98d480",
    "lcode": "luaK_checkstack@0x9a6a20 luaK_reserveregs@0x9a6a60 luaK_code@0x9a6e90 luaK_codeABC@0x9a6f30 "
             "luaK_jump@0x9a70c0 luaK_ret@0x9a7100 constfolding@0x9a6d70 luaK_codeABx@0x9a6f60 "
             "luaK_nil@0x9a7020 fixjump@0x9a6740 patchlistaux@0x9a6910",
}


def cut(s, start, end_incl):
    a = s.index(start)
    b = s.index(end_incl, a) + len(end_incl)
    return s[:a], s[b:]


def prepare(d, a):
    for f in os.listdir(SRC):
        if f.endswith((".c", ".h")):
            shutil.copy(os.path.join(SRC, f), d)
    p = os.path.join(d, "luaconf.h")
    s = open(p).read()
    head, tail = cut(s, '#include "boost/shared_ptr.hpp"', '#include "RBX/Debug.h"')
    s = head + '#include "rbxstub.h"' + tail
    head, tail = cut(s, "#pragma pack(push)", "#pragma pack(pop)")
    s = head + """#pragma pack(push, 8)
class RobloxExtraSpace { /* probe stub, 40 bytes like PLAYER */
public:
  void *hook[3]; void *shared[2]; void *node; unsigned int identity:5; unsigned int yieldCaptured:1;
  void *script[2]; void *continuations;
  static RobloxExtraSpace* get(struct lua_State *L) { return L ? reinterpret_cast<RobloxExtraSpace*>(((char*) L) - sizeof(RobloxExtraSpace)) : 0; }
  static void constructRoot(lua_State *) {} static void destroyRoot(lua_State *) {}
  static void constructChild(lua_State *, RobloxExtraSpace *) {} static void destroyChild(lua_State *) {}
};
#pragma pack(pop)""" + tail
    head, tail = cut(s, "inline void lua_vmhooked_handler_ex", "\n}")
    s = head + "inline void lua_vmhooked_handler_ex(lua_State* L) { lua_vmhooked_handler(L); }" + tail
    open(p, "w").write(s)
    vm = open(LUAVM_H).read()
    macros = vm[vm.index("// Utilities for shuffling"):vm.index("// Utility class\nstruct lua_State;")]
    open(os.path.join(d, "rbxstub.h"), "w").write(
        "typedef unsigned int uint32_t; typedef int int32_t; typedef unsigned __int64 uint64_t;"
        " typedef unsigned char uint8_t;\n#include <exception>\n#define RBXASSERT(x) ((void)0)\n"
        "namespace RBX { class base_exception : public std::exception {}; class BaseScript; class ScriptContext; }\n"
        + macros + "#define LUAVM_KEY_DUMMY 1\n#define LUAVM_KEY_INVALID 0\n#define LUAVM_MODKEY_DUMMY 1\n"
        + ("" if a.no_xmmintrin else "#include <xmmintrin.h>\n"))
    p = os.path.join(d, "lapi.c")
    s = open(p).read().replace('#include "V8DataModel/', '// probe: #include "V8DataModel/')
    open(p, "w").write(s)
    if not a.keep_wrappers:
        for h in ("lobject.h", "lstate.h"):
            p = os.path.join(d, h)
            s = re.sub(r"LuaVMValue<([^>\n]*)>", r"\1", open(p).read())
            open(p, "w").write(s)
    if not a.keep_ckey:
        p = os.path.join(d, "lstate.h")
        s = open(p).read()
        s, n = re.subn(r"LUAVM_SHUFFLE8\(;,\n  (?:LuaVMValue<unsigned int>|unsigned int) ckey,\n *\n",
                       "LUAVM_SHUFFLE7(;,\n", s)
        assert n == 1, "ckey block not found"
        open(p, "w").write(s)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--flags", default=FLAGS)
    ap.add_argument("--no-xmmintrin", action="store_true")
    ap.add_argument("--keep-wrappers", action="store_true")
    ap.add_argument("--keep-ckey", action="store_true")
    ap.add_argument("-v", action="store_true", help="print masked diffs")
    a = ap.parse_args()
    os.makedirs("/tmp/claude-1000", exist_ok=True)
    d = tempfile.mkdtemp(prefix="ogre-luaprobe-", dir="/tmp/claude-1000")
    prepare(d, a)
    cl = os.path.join(ROOT, "tools/msvc9/cl")
    total = same = 0
    for tu, pairs in PAIRS.items():
        obj = os.path.join(d, tu + ".obj")
        r = subprocess.run([cl] + COMMON + a.flags.split() + ["/Fo" + obj, os.path.join(d, tu + ".c")],
                           capture_output=True, text=True, timeout=300)
        if r.returncode:
            sys.exit(r.stdout)
        r = subprocess.run([sys.executable, "-P", os.path.join(ROOT, "tools/probe_compare.py"), obj]
                           + pairs.split() + ([] if a.v else ["-q"]), capture_output=True, text=True)
        print(r.stdout, end="")
        lines = [l for l in r.stdout.splitlines() if "IDENTICAL" in l or "DIFFERENT" in l]
        total += len(lines)
        same += sum("IDENTICAL" in l for l in lines)
    print(f"{same}/{total} identical  flags: {a.flags}  scratch: {d}")
    return 0 if same == total else 1


if __name__ == "__main__":
    sys.exit(main())
