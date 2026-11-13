# Project OGRE: matching decompilation of the October 2013 Roblox Windows clients.
#
#   python3 -P tools/lib/install_libs.py      # third-party sources from staging/ (RakNet and G3D are gitignored)
#   cmake -G Ninja -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/msvc9-wine.cmake
#   cmake --build build
#   python3 -P tools/ida/gen_reccmp_csv.py PLAYER  # library rows are resolved against build/'s map and PDB
#   cd build && reccmp-reccmp --target PLAYER --nolib
# STUDIO (its own 2013 checkout path in every assert string, so its own objects; cmake/msvc9-wine.cmake):
#   cmake -G Ninja -S . -B build-studio -DCMAKE_TOOLCHAIN_FILE=cmake/msvc9-wine.cmake -DOGRE_TARGET=STUDIO
#   cmake --build build-studio --target studio
#   python3 -P tools/ida/gen_reccmp_csv.py STUDIO --build build-studio
#   cd build-studio && reccmp-reccmp --target STUDIO --nolib
#
# One OBJECT library per original 2013 static library, under Client/ at the paths the 2013 binaries'
# own __FILE__ strings record (notes/paths-2013.txt). Each library carries its own flags, because
# each was compiled with different ones. A flag is either proven (the function that proved it is
# named next to it) or marked "TODO prove" and taken from the 2016 .vcxproj Release|Win32 values.
#
# Not here, on purpose:
#  - Log: it ships as Log.dll beside both exes (bin/player, bin/studio), not as a static library of
#    either; it needs its own DLL target and its own reccmp target.
#  - OgreMain, tbb, fmodex, cg, Qt: separate DLLs. (boost.dll is built below: the exes link its
#    import library.)
#  - D3DX9 (the 13.10.4035 objects at the tail of .text): a prebuilt SDK library, not ours to compile.
cmake_minimum_required(VERSION 3.25) # CMP0141: CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
project(OGRE C CXX)

# Bound memory: several agents build at once, and each wine cl.exe of a big App TU takes a few
# hundred MB. Ninja's default (cores + 2 jobs per build) ran the machine out of memory on 2026-11-12.
set(OGRE_COMPILE_JOBS 6 CACHE STRING "Parallel compile jobs per build tree")
set_property(GLOBAL APPEND PROPERTY JOB_POOLS ogre_compile=${OGRE_COMPILE_JOBS} ogre_link=1)
set(CMAKE_JOB_POOL_COMPILE ogre_compile)
set(CMAKE_JOB_POOL_LINK ogre_link)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/reccmp.cmake")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Only VC9 SP1 15.00.30729.01 can match (Rich header). Other compilers may build but never score.
if(MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "15.0.30729.1")
    set(MSVC_FOR_DECOMP TRUE)
else()
    set(MSVC_FOR_DECOMP FALSE)
endif()
message(STATUS "MSVC for decompilation: ${MSVC_FOR_DECOMP}")

# Every flag is set per library below, so CMake's defaults (/O2 /Ob2 /W3 /GR /EHsc /DNDEBUG ...)
# must not leak in. Runtime /MD and /Z7 come from the two CMAKE_MSVC_* properties.
foreach(lang C CXX)
    set(CMAKE_${lang}_FLAGS "/nologo")
    foreach(cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
        set(CMAKE_${lang}_FLAGS_${cfg} "")
    endforeach()
endforeach()
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)     # /MD: both exes import MSVCR90/MSVCP90
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded)    # /Z7: no shared compile PDB, no mspdbsrv races
foreach(cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_EXE_LINKER_FLAGS_${cfg} "")
endforeach()

# --- Flag sets --------------------------------------------------------------------------------------

# App, proven 2026-11-12: 52/52 Lua functions byte-identical (tools/msvc9/lua_probe.py).
#   /Oy (not the 2016 /Oy-): luaO_rawequalObj 0x008a7940 has no ebp frame.
#   /arch:SSE (not the 2016 /arch:SSE2): luaH_getnum 0x0098c480 compares with fld/fucomip.
set(OGRE_APP_FLAGS /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /D_SECURE_SCL=0
    /DNDEBUG /DWIN32 /D_LIB /D_RELEASE)

# Per-library flags below were decided 2026-11-12 by byte probes (tools/probe_flags.py, sources in
# tools/flagprobes/, table in notes/flags-proof.md): a probe function is byte-identical under the set,
# and flipping the one flag loses the function named next to it. "census" = whole-range statistic only
# (tools/probe_libs.py). _SECURE_SCL=0 everywhere: RBX::Poly::getNumSurfaces 0x00754610 (lost under =1).

# Base: the App set, all PLAYER addresses (STUDIO twins in notes/flags-proof.md).
#   /Oy and /O2: format 0x009b5ba0, ExclusiveArbiter::arbiterName 0x009b3950 (lost under /Oy-, /O1).
#   /Ob2: Time::nowFastSec 0x009bb4d0 (lost under /Ob1). /GS-: c_time::localtime 0x009b1240,
#   Log::formatMem 0x009b0f50. /arch:SSE: Log::formatTime 0x009b10e0 (lost with no /arch and under
#   SSE2), Job::computeStandardError 0x009b98b0 (SSE2). /fp:fast: Log::formatMem 0x009b0f50.
#   /Oi: census only (no strlen/strcmp import). TODO prove.
#   The _WIN32_WINNT/NTDDI_VERSION/_CRT_SECURE_NO_WARNINGS defines are 2016 values. TODO prove.
set(OGRE_BASE_FLAGS /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /D_SECURE_SCL=0
    /DWIN32 /DNDEBUG /D_LIB /D_WIN32_WINNT=0x0501 /DNTDDI_VERSION=0x05010100 /D_CRT_SECURE_NO_WARNINGS)

# Network, which also compiles RakNet 4.036. Proven = the App set: 566 of 567 RakNet functions
# identical (tools/lib/raknet_match.py); /Oy- keeps 108, /arch:SSE2 496, /GS 424. /fp:fast:
# RakNet::BitStream::ReadFloat16 0x005c3910; /arch:SSE: RakNet::SystemAddress::operator= 0x005710d0.
set(OGRE_NETWORK_FLAGS ${OGRE_APP_FLAGS} /D_CRT_SECURE_NO_WARNINGS)

# Rendering/AppDraw: the App set. CircleRadialNormal::eval 0x009c2900, evalTangent 0x009c2990,
#   evalNormal 0x009c2a10 decide /Oy (2016 said /Oy-; all 4 probes lost), /arch:SSE (lost with none
#   and under SSE2), /fp:fast, /Oi, /O2. /Ob: lost under /Ob0, /Ob1 vs /Ob2 TODO prove.
#   /GS-: census only (0 cookies, 5 EH frames). TODO prove. /GF /Gy- (2016) and RBX_TEST_BUILD: TODO prove.
set(OGRE_APPDRAW_FLAGS /O2 /Ob2 /Oi /Oy /GF /Gy- /GS- /arch:SSE /fp:fast /GR /EHsc /D_SECURE_SCL=0
    /DWIN32 /DNDEBUG /D_LIB /D_RELEASE /DRBX_TEST_BUILD)

# Rendering/GfxBase: unoptimized. /Od: all 7 probes lost under /O2 (CRenderSettings::
#   setAutoQualityLevel 0x009c5ed0, ViewBase::render 0x009c7c10, AdornBillboarder::drawFont2DImpl
#   0x009d5720, ...). /Ob2 (or /Ob1, not /Ob0): the inlined runtime_error ctor, STUDIO 0x00aa1ff0.
#   /GS-: 0x009d5720. /arch:SSE: 0x009c7c10. /Oy-, /fp, /Oi have no effect under /Od (2016 values
#   kept). /GF and RBX_TEST_BUILD: TODO prove.
set(OGRE_GFXBASE_FLAGS /Od /Ob2 /Oi /Oy- /GF /GS- /arch:SSE /fp:fast /GR /EHsc /D_SECURE_SCL=0
    /DWIN32 /DNDEBUG /D_LIB /DRBX_TEST_BUILD)

# Rendering/RbxG3D: census only (its one PLAYER function, the Frustum ctor 0x009ee010: EH frame and
# no cookie = /GS-, ebp as a general register = /Oy, 70 SSE1 and no SSE2). /Ob, /fp, SSE vs SSE2,
# /GF /Gy- (2016): TODO prove.
set(OGRE_RBXG3D_FLAGS /O2 /Ob2 /Oi /Oy /GF /Gy- /GS- /arch:SSE /fp:fast /GR /EHsc /D_SECURE_SCL=0
    /DWIN32 /DNDEBUG /D_LIB)

# Rendering/g3d: G3D 8.00 with its bundled zlib 1.2.3 and libzip 0.9 (and IJG 6b, compiled only
# to resolve GImage_jpeg's externals, nothing of them is linked in PLAYER). Proven (tools/lib/g3d_match.py,
# zlib 45/45, libzip 65/65, G3D 161 identical): the App set, C files as C. Each flip loses functions:
# /Oy- 233 (_zcfree 0x009eb660), /arch:SSE2 65 (_crc32@12 0x009e0700), no /arch 79 (_flush_pending
# 0x009e0750), /fp:precise 53 (G3D::fnan 0x009d7e50), /GS 21 (_gen_codes 0x009eae50). ZLIB_WINAPI:
# _deflate@8 0x009e07a0 ends in ret 8. /Gy (implied by /O2): deflateInit_ absent, deflateInit2_ linked.
# The 2016 /Oy-, /arch:SSE2, /Gy- and G3D_BUILDING_LIBRARY_DLL are all wrong for 2013.
set(OGRE_G3D_FLAGS ${OGRE_APP_FLAGS} /DZLIB_WINAPI /D_CRT_SECURE_NO_WARNINGS)

# boost 1.53.0 (tools/lib/boost_match.py). The static libraries in the exe (filesystem, iostreams,
# program_options) are the App set with /GS on: boost::iostreams::zlib_error::check 0x006130e0 has
# the cookie; /EHsc, not boost.build's /EHs: codecvt_error_category 0x009c0f20. System and thread
# come from boost.dll (BOOST_*_DYN_LINK); zlib calls are stdcall (zlib_base::do_init 0x00613170).
set(OGRE_BOOST_DEFS /DWIN32 /DNDEBUG /D_LIB /D_SECURE_SCL=0 /DZLIB_WINAPI /D_CRT_SECURE_NO_WARNINGS
    /D_WIN32_WINNT=0x0501 /DNTDDI_VERSION=0x05010100 /DBOOST_ALL_NO_LIB)
set(OGRE_BOOST_FLAGS /O2 /Ob2 /Oi /Oy /arch:SSE /fp:fast /GR /EHsc ${OGRE_BOOST_DEFS}
    /DBOOST_SYSTEM_DYN_LINK /DBOOST_THREAD_DYN_LINK)
# boost.dll (thread + system): the App set (/GS-: 47/47 out-of-line functions; /GS gives 35/44).
# BOOST_SYSTEM_SOURCE in every TU: boost::thread::start_thread calls system_category directly
# (boost.dll 0x10011751). BOOST_CHRONO_HEADER_ONLY: try_join_until 0x10014880 inlines system_clock::now.
set(OGRE_BOOSTDLL_FLAGS /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc ${OGRE_BOOST_DEFS}
    /DBOOST_SYSTEM_DYN_LINK /DBOOST_SYSTEM_SOURCE /DBOOST_THREAD_BUILD_DLL /DBOOST_CHRONO_HEADER_ONLY
    /D_USRDLL /D_WINDLL)

# Rendering/RbxOgre and Rendering/GfxAdapters: 2013 only (the 2016 tree replaced them with
# GfxCore/GfxRender), no .vcxproj. Both are the App set.
#   RbxOgre: istreamDataStream::read 0x0051f270 (/Oy, /Ob), Name::doDeclare<sDecal> 0x00480bf0 (/GS-),
#   AdornRbxGfx::getViewport 0x00491700 (/arch:SSE over none, /fp:fast), FrameRateManagerStatsItem::
#   update 0x0048e530 (SSE over SSE2). /Oi: census only, TODO prove.
#   GfxAdapters: boost unordered_set<GfxPart*> swap 0x0060b930 (/Ob2 over /Ob1, /Oy, SSE over none,
#   /O2), table_impl copy ctor 0x0060cd30 (/GS-). SSE vs SSE2 and /fp: census only, TODO prove.
set(OGRE_RBXOGRE_FLAGS ${OGRE_APP_FLAGS})
set(OGRE_GFXADAPTERS_FLAGS ${OGRE_APP_FLAGS})

# The client shells are built unlike the engine: /GS, /fp:precise, x87 only (no /arch), and /GL, so
# the exes link /LTCG. The 2016 /GS- and /arch:SSE2 are both wrong for 2013.
# WindowsClient (also compiles Client/ClientBase, Client/Win, ClientShared, and in 2013 boost
# program_options, which is LTCG in PLAYER at 0x00478330). Unnamed PLAYER probes:
#   /GS: Name::doDeclare<sScript> 0x00413740. /fp:precise: 0x0040bfa0. no /arch: 0x00429ce0 and
#   0x0040bfa0 (lost under /arch:SSE). /Oy and /O2: all three. /GL: census (regarg 12.9-19.9%) plus the
#   Rich header's 40 LTCG objects (= this project's 44 TUs incl. program_options). /Ob2 /Oi: TODO prove
#   (/Ob0 and /Oi- still match). BOOST_THREAD_BUILD_LIB, WINVER, _WIN32_WINNT: 2016 values, TODO prove.
set(OGRE_WINDOWSCLIENT_FLAGS /O2 /Ob2 /Oi /Oy /GS /fp:precise /GL /GR /EHsc /D_SECURE_SCL=0
    /DBOOST_THREAD_BUILD_LIB /DWIN32 /DNDEBUG /D_WINDOWS /DWINVER=0x0501 /D_WIN32_WINNT=0x0501)

# RobloxStudio, Qt 4.8.5. Also compiles ClientBase, ClientShared and Win. STUDIO addresses:
#   /Oi: moc UpdateUIManager::qt_metacast 0x0050eca0 (inline strcmp). /Ob>0: metaObject 0x0050ec50.
#   /Oy: 0x0050eca0, qt_metacall 0x0050ed00, FrameRateManagerStatsItem::update 0x0043e8a0. /O2: all 4.
#   /GS: 0x0047eb90. /fp:precise: 0x004780e0. no /arch: 0x00486610, 0x004780e0. /GL: census plus the
#   Rich header's 127 LTCG objects. QT_NO_DEBUG is not defined: Q_ASSERT survives in 0x0050ec50.
#   The other Qt and Roblox defines are 2016 values, TODO prove.
set(OGRE_STUDIO_FLAGS /O2 /Ob2 /Oi /Oy /GS /fp:precise /GL /GR /EHsc /D_SECURE_SCL=0
    /DBOOST_THREAD_BUILD_LIB /DRBX_STUDIO_BUILD /DWIN32 /DNDEBUG /DQT_THREAD_SUPPORT /DQT_NO_KEYWORDS
    /DQT_DLL /DQT_GUI_LIB /DQT_CORE_LIB /DQT_WEBKIT_LIB /DQT_ROBLOX_STUDIO)

# --- Libraries --------------------------------------------------------------------------------------

function(ogre_library NAME)
    cmake_parse_arguments(ARGS "" "" "SOURCES;FLAGS" ${ARGN})
    add_library(${NAME} OBJECT ${ARGS_SOURCES})
    target_include_directories(${NAME} PRIVATE "${PROJECT_SOURCE_DIR}/util")
    if(MSVC_FOR_DECOMP)
        target_compile_options(${NAME} PRIVATE ${ARGS_FLAGS})
    else()
        target_compile_options(${NAME} PRIVATE /EHsc)
    endif()
endfunction()

# Third-party sources are installed from staging/ (upstream + patches) by tools/lib/install_libs.py.
# Their functions are paired by reccmp/<t>-<lib>.csv rows (decorated symbol + PDB name), not markers.
set(OGRE_DL "${PROJECT_SOURCE_DIR}/3rdparty/_downloads")
set(OGRE_BOOST_INCLUDE "${OGRE_DL}/boost_1_53_0/boost_1_53_0")
set(OGRE_G3D "${PROJECT_SOURCE_DIR}/Client/Rendering/g3d")

# Lua 5.1.4 is part of App in 2013 and is compiled as C++ with the App flags (514/514 identical,
# tools/lib/lua_match.py). Its own target only so that App's include roots never reach it: the
# patches' stand-ins (Client/App/Lua-5.1.4/standin: FastLog.h, RbxProtectedString.h; patch 01's
# <intrin.h> for rbx/Debug.h -> RbxPlatform.h; patch 10's RobloxExtraSpace declaration) replace the
# real Log/Base headers, which are not 2013-exact yet. ldblib, liolib, loslib, loadlib and linit have
# no body and no strings in PLAYER or STUDIO; lua.c, luac.c and print.c are the stand-alone tools.
# The stand-ins are a source-level include directory (searched before App's own include roots), so
# they reach only the Lua TUs, never the rest of App.
set(OGRE_LUA_TUS lapi lauxlib lbaselib lcode ldebug ldo ldump lfunc lgc llex lmathlib lmem lobject
    lopcodes lparser lstate lstring lstrlib ltable ltablib ltm lundump lvm lzio)
list(TRANSFORM OGRE_LUA_TUS REPLACE "(.+)" "Client/App/Lua-5.1.4/src/\\1.c" OUTPUT_VARIABLE OGRE_LUA_SOURCES)
set_source_files_properties(${OGRE_LUA_SOURCES} PROPERTIES LANGUAGE CXX
    INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/Client/App/Lua-5.1.4/standin;${OGRE_BOOST_INCLUDE}")

# RakNet 4.036 + Roblox 2013 edits: the 26 TUs of Client/Network/raknet/Source (25 linked in PLAYER,
# plus RakMemoryOverride, whose __FILE__ is in PLAYER).
file(GLOB OGRE_RAKNET_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/Client/Network/raknet/Source/*.cpp")

# G3D 8.00 (Roblox fork + patches) with zlib 1.2.3 (zlib/), libzip 0.9 (zip/) and IJG 6b (ijg/). Directory names for zlib and zip are the 2016 tree's (zip/, include/zip);
# no 2013 __FILE__ records them.
# PLAYER links no libpng, so GImage_png.cpp is left out and shim/ resolves GImage.cpp's references.
file(GLOB OGRE_G3D_CPP CONFIGURE_DEPENDS "${OGRE_G3D}/G3Dcpp/*.cpp")
list(FILTER OGRE_G3D_CPP EXCLUDE REGEX "/GImage_png\\.cpp$")
list(APPEND OGRE_G3D_CPP "${OGRE_G3D}/shim/png_absent.cpp")
file(GLOB OGRE_G3D_C CONFIGURE_DEPENDS "${OGRE_G3D}/zlib/*.c" "${OGRE_G3D}/zip/*.c" "${OGRE_G3D}/ijg/*.c")

ogre_library(decomp_util SOURCES util/decomp.cpp FLAGS ${OGRE_APP_FLAGS})
ogre_library(App SOURCES ${OGRE_LUA_SOURCES} FLAGS ${OGRE_APP_FLAGS})
ogre_library(Base SOURCES Client/Base/placeholder.cpp FLAGS ${OGRE_BASE_FLAGS})
ogre_library(Network SOURCES Client/Network/placeholder.cpp ${OGRE_RAKNET_SOURCES} FLAGS ${OGRE_NETWORK_FLAGS})
# RakNet's Roblox edits (patch 0002) use FastLog.h from the Log library.
target_include_directories(Network PRIVATE "${PROJECT_SOURCE_DIR}/Client/Log/include")
ogre_library(AppDraw SOURCES Client/Rendering/AppDraw/placeholder.cpp FLAGS ${OGRE_APPDRAW_FLAGS})
ogre_library(GfxBase SOURCES Client/Rendering/GfxBase/placeholder.cpp FLAGS ${OGRE_GFXBASE_FLAGS})
ogre_library(GfxAdapters SOURCES Client/Rendering/GfxAdapters/placeholder.cpp FLAGS ${OGRE_GFXADAPTERS_FLAGS})
ogre_library(RbxG3D SOURCES Client/Rendering/RbxG3D/placeholder.cpp FLAGS ${OGRE_RBXG3D_FLAGS})
ogre_library(RbxOgre SOURCES Client/Rendering/RbxOgre/placeholder.cpp FLAGS ${OGRE_RBXOGRE_FLAGS})
ogre_library(g3d SOURCES ${OGRE_G3D_CPP} ${OGRE_G3D_C} FLAGS ${OGRE_G3D_FLAGS})
# G3D's own include roots (tools/lib/g3d_match.py); Rendering for G3D/CoordinateFrame.h's RbxG3D include,
# Log for Quat.cpp's FastLog.h, vc9compat for <stdint.h>.
target_include_directories(g3d PRIVATE "${OGRE_G3D}/include" "${OGRE_G3D}/zlib" "${OGRE_G3D}/zip"
    "${OGRE_G3D}/ijg" "${OGRE_G3D}/shim" "${PROJECT_SOURCE_DIR}/Client/Rendering/RbxG3D/include"
    "${PROJECT_SOURCE_DIR}/Client/Log/include" "${PROJECT_SOURCE_DIR}/Client/Rendering"
    "${PROJECT_SOURCE_DIR}/3rdparty/vc9compat"
    # The fork's CoordinateFrame.h includes "../../../App.BulletPhysics/LinearMath/btTransform.h"
    # (a 2016 addition), which resolves only against the prior tree. Bullet is not in any 2013
    # build, but its btScalar.h makes <emmintrin.h> visible, which changes static-function register
    # conventions, and the G3D match (161 identical) was measured with it. Read only, never edited.
    "${PROJECT_SOURCE_DIR}/2013 55% App/Rendering/RbxG3D/include")

# boost 1.53.0 library sources (pristine, 3rdparty/boost_1_53_0) against the downloaded headers.
set(OGRE_BOOST_SRC "${PROJECT_SOURCE_DIR}/3rdparty/boost_1_53_0/libs")
set(OGRE_BOOST_STATIC_SOURCES)
foreach(f filesystem/src/codecvt_error_category filesystem/src/operations filesystem/src/path
        filesystem/src/path_traits filesystem/src/portability filesystem/src/unique_path
        filesystem/src/utf8_codecvt_facet filesystem/src/windows_file_codecvt
        iostreams/src/file_descriptor iostreams/src/gzip iostreams/src/mapped_file iostreams/src/zlib
        program_options/src/cmdline program_options/src/config_file program_options/src/convert
        program_options/src/options_description program_options/src/parsers
        program_options/src/positional_options program_options/src/split
        program_options/src/utf8_codecvt_facet program_options/src/value_semantic
        program_options/src/variables_map program_options/src/winmain)
    list(APPEND OGRE_BOOST_STATIC_SOURCES "${OGRE_BOOST_SRC}/${f}.cpp")
endforeach()
ogre_library(boost_static SOURCES ${OGRE_BOOST_STATIC_SOURCES} FLAGS ${OGRE_BOOST_FLAGS})
target_include_directories(boost_static PRIVATE "${OGRE_BOOST_INCLUDE}" "${OGRE_G3D}/zlib")

# boost.dll: Boost.Thread + Boost.System, a DLL beside both exes; the exes link its import library.
add_library(boost SHARED "${OGRE_BOOST_SRC}/system/src/error_code.cpp" "${OGRE_BOOST_SRC}/thread/src/win32/thread.cpp"
    "${OGRE_BOOST_SRC}/thread/src/win32/tss_dll.cpp" "${OGRE_BOOST_SRC}/thread/src/win32/tss_pe.cpp"
    "${OGRE_BOOST_SRC}/thread/src/tss_null.cpp" "${OGRE_BOOST_SRC}/thread/src/future.cpp")
target_include_directories(boost PRIVATE "${OGRE_BOOST_INCLUDE}")
if(MSVC_FOR_DECOMP)
    target_compile_options(boost PRIVATE ${OGRE_BOOSTDLL_FLAGS})
endif()
target_link_options(boost PRIVATE /DEBUG /INCREMENTAL:NO)

# Sources the client projects compile themselves, once per client with that client's flags.
set(OGRE_CLIENT_COMMON_SOURCES Client/ClientBase/placeholder.cpp Client/Win/placeholder.cpp)
ogre_library(player_client SOURCES Client/WindowsClient/placeholder.cpp ${OGRE_CLIENT_COMMON_SOURCES}
    FLAGS ${OGRE_WINDOWSCLIENT_FLAGS})
ogre_library(studio_client SOURCES Client/RobloxStudio/placeholder.cpp ${OGRE_CLIENT_COMMON_SOURCES}
    Client/ClientShared/placeholder.cpp FLAGS ${OGRE_STUDIO_FLAGS})

set(OGRE_ENGINE_LIBS decomp_util App Base Network AppDraw GfxBase GfxAdapters RbxG3D RbxOgre g3d boost_static)

# --- Prior App decomp (Phase 2 step 7) ----------------------------------------------------------------
# The user's earlier 2013 App reconstruction, copied into Client/ by tools/import_prior_app.py and
# annotated by tools/convert_app_markers.py. ON since 2026-11-12: it links into both exes (unwritten
# callees through Client/_generated stubs or /FORCE:UNRESOLVED). OFF builds only the libraries. 0-byte files are the prior tree's
# TODO map and are not compiled. Network/raknet and Rendering/g3d hold the staged 2013 libraries
# (tools/lib/install_libs.py), compiled above with their own include roots, so the prior globs leave
# them out (Rendering/g3d entirely: its G3Dcpp/GImage_png.cpp has no libpng to compile against).
option(OGRE_PRIOR_APP "Compile the imported prior App decomp" ON)
# Defined here, before Log.dll reads it; explained at ogre_executable below.
option(OGRE_FORCE_UNRESOLVED "Link despite unresolved externals (LNK4088)" ON)

# Log ships as Log.dll beside both exes. Probes on bin/player/Log.dll exports: /Oy and /O2:
# FLog::FastLog(3 args) 0x10002770 and 5 more lost under /Oy-; /Ob>0 and no /arch: FastLogF 0x10002940
# (lost under /arch:SSE). /GS: census (94 cookies over 86 EH frames). /GL: the Rich header has exactly
# 2 LTCG objects (FastLog.cpp, FastLogStream.cpp). /Oi: inline strlen in FastLogS 0x10002b30, not
# probed. /fp: TODO prove (2016 /fp:fast kept out: no /arch means x87, precise is the VC9 default).
set(OGRE_LOG_FLAGS /O2 /Ob2 /Oi /Oy /GS /GL /GR /EHsc /D_SECURE_SCL=0 /DNDEBUG /DWIN32)

# Third-party header roots for the prior tree. The prior harness used a 2016-era Contribs that is not
# here. boost 1.53.0 is the pinned version. TBB: the 2013 tbb.dll is TBB 2.1 (interface 3011), which
# has no official download any more; 4.4 is the oldest upstream tag, used only for headers
# (queuing_rw_mutex). 3rdparty/vc9compat: headers VC9 lacks (stdint.h, cstdint). Lua: Client/App/Lua-5.1.4/src first (the 2013 Lua agent's home); the prior
# tree's own 2016-fork headers are the fallback until that agent's headers land there.
set(OGRE_DL "${PROJECT_SOURCE_DIR}/3rdparty/_downloads")
set(OGRE_PRIOR_LUA_FALLBACK "${PROJECT_SOURCE_DIR}/2013 55% App/App/Lua-5.1.4/src" CACHE PATH
    "Lua headers used after Client/App/Lua-5.1.4/src")
set(OGRE_PRIOR_INCLUDES
    Client/App/include Client/App/Lua-5.1.4/src "${OGRE_PRIOR_LUA_FALLBACK}" Client/Base/include
    Client/Network/include Client/Rendering/g3d/include Client/Rendering/RbxG3D/include
    Client/Rendering/GfxBase/include Client/Log/include
    "${OGRE_DL}/boost_1_53_0/boost_1_53_0" "${OGRE_DL}/tbb-4.4/oneTBB-4.4/include"
    "${OGRE_DL}/zlib-1.2.3/zlib-1.2.3" 3rdparty/vc9compat)
list(TRANSFORM OGRE_PRIOR_INCLUDES PREPEND "${PROJECT_SOURCE_DIR}/" REGEX "^3rdparty/")
list(TRANSFORM OGRE_PRIOR_INCLUDES PREPEND "${PROJECT_SOURCE_DIR}/" REGEX "^Client/")

# Adds every non-empty file matching GLOBS (relative to Client/) to LIB, minus EXCLUDE regexes.
function(ogre_prior_sources LIB)
    cmake_parse_arguments(ARGS "" "" "GLOBS;EXCLUDE" ${ARGN})
    list(TRANSFORM ARGS_GLOBS PREPEND "${PROJECT_SOURCE_DIR}/Client/")
    file(GLOB_RECURSE found CONFIGURE_DEPENDS ${ARGS_GLOBS})
    list(FILTER found EXCLUDE REGEX "/placeholder\\.cpp$")
    foreach(re ${ARGS_EXCLUDE})
        list(FILTER found EXCLUDE REGEX "${re}")
    endforeach()
    set(keep)
    foreach(f ${found})
        file(SIZE "${f}" n)
        if(n GREATER 0)
            list(APPEND keep "${f}")
        endif()
    endforeach()
    target_sources(${LIB} PRIVATE ${keep})
    # App's include root as each 2013 project spelled its /I, which header asserts record
    # (C:\BuildAgent\work\<hash>\Client\App\Include\Util/... in RbxOgre code, Client\app\include in
    # GfxBase, Client\App\include elsewhere; tools/msvc9/winewrap.py). Wine resolves the case.
    set(incs ${OGRE_PRIOR_INCLUDES})
    if(LIB STREQUAL "RbxOgre")
        list(TRANSFORM incs REPLACE "/Client/App/include$" "/Client/App/Include")
    elseif(LIB STREQUAL "GfxBase")
        list(TRANSFORM incs REPLACE "/Client/App/include$" "/Client/app/include")
    endif()
    target_include_directories(${LIB} PRIVATE ${incs})
    # Warning and auto-link switches only, no codegen effect (the prior harness defined them too).
    # ROBLOX_BOOST_CONFIGS: Roblox's boost/config/user.hpp defines only this (rbx/boost.hpp checks it).
    # BOOST_THREAD/SYSTEM_DYN_LINK: PLAYER calls all 18 boost.dll imports (IAT 0x00aa2d00-0x00aa2d44) as
    # `call [imp]` from client, Network, App and Base (146 sites, no jmp thunk). Security::Context::current
    # 0x0053ac40 matches only with it.
    target_compile_definitions(${LIB} PRIVATE BOOST_ALL_NO_LIB _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS
        ROBLOX_BOOST_CONFIGS BOOST_THREAD_DYN_LINK BOOST_SYSTEM_DYN_LINK)
endfunction()

if(OGRE_PRIOR_APP)
    # Log.dll (FLog/FFlag), imported by both exes. Its own DLL, as in 2013, so the exes import FLog::*
    # the way the originals do (PLAYER's IAT names 30+ Log.dll slots from 0x00aa12a4).
    add_library(Log SHARED)
    if(MSVC_FOR_DECOMP)
        target_compile_options(Log PRIVATE ${OGRE_LOG_FLAGS})
    endif()
    target_compile_definitions(Log PRIVATE LOG_DLL)
    target_link_libraries(Log PRIVATE boost ws2_32)
    target_link_options(Log PRIVATE /DEBUG /INCREMENTAL:NO /LTCG)
    if(OGRE_FORCE_UNRESOLVED)
        target_link_options(Log PRIVATE /FORCE:UNRESOLVED)
    endif()
    # Kept out of the build (files stay in the tree):
    # - util/StreamRegion.cpp: 2013 code (iOS 1029 has both BitStream operators), but its headers are
    #   2016 copies or stubs: Util/StreamRegion.h needs 2016 SpatialHashStatic/ContactManagerSpatialHash
    #   APIs, Voxel/Cell.h is a 0-byte stub, Util/StreamingUtil.h does not exist.
    ogre_prior_sources(App GLOBS App/*.cpp App/*.c EXCLUDE "/App/Lua-5\\.1\\.4/"
        "/App/util/StreamRegion\\.cpp$")
    # These three use RobloxExtraSpace's members (context(), getThreadCount(), script, continuations),
    # which the 2013 Lua's luaconf.h keeps opaque (only the 44-byte layout PLAYER proves). They see the
    # prior tree's full class (2016-fork Lua headers) instead: a source-level include directory comes
    # before the target's. TODO: a 2013 luaconf.h that gives App the full class.
    set_source_files_properties("${PROJECT_SOURCE_DIR}/Client/App/script/ScriptContext.cpp"
        "${PROJECT_SOURCE_DIR}/Client/App/script/ThreadRef.cpp"
        "${PROJECT_SOURCE_DIR}/Client/App/script/LuaInstanceBridge.cpp"
        PROPERTIES INCLUDE_DIRECTORIES "${OGRE_PRIOR_LUA_FALLBACK}")
    # lz4 upstream svn r94 (2013-04-26), the revision the 2016 tree's App/lz4 carries. PLAYER has the
    # decoder (SerializerBinary deserialize, "Malformed data (%d != %d)"); headers only for now.
    target_include_directories(App PRIVATE "${OGRE_DL}/lz4-r94/lz4-r94")
    ogre_prior_sources(Base GLOBS Base/*.cpp)
    ogre_prior_sources(Log GLOBS Log/*.cpp)
    ogre_prior_sources(Network GLOBS Network/*.cpp EXCLUDE "/Network/raknet/")
    ogre_prior_sources(GfxBase GLOBS Rendering/GfxBase/*.cpp)
    ogre_prior_sources(RbxG3D GLOBS Rendering/RbxG3D/*.cpp)
    ogre_prior_sources(RbxOgre GLOBS Rendering/RbxOgre/*.cpp)
endif()

# --- Executables ------------------------------------------------------------------------------------

# tbb.lib: TBB's headers auto-link it, and the originals import 4 names from tbb.dll (TBB 2.1, no
# official download). Built from the shipped DLL's export table (identical in bin/player and bin/studio).
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/tbb.lib"
    COMMAND python3 -P "${PROJECT_SOURCE_DIR}/tools/dll_implib.py" "${PROJECT_SOURCE_DIR}/bin/player/tbb.dll"
        "${CMAKE_BINARY_DIR}/tbb.def"
    COMMAND "${CMAKE_AR}" /nologo "/def:${CMAKE_BINARY_DIR}/tbb.def" /machine:x86 "/out:${CMAKE_BINARY_DIR}/tbb.lib"
    DEPENDS "${PROJECT_SOURCE_DIR}/tools/dll_implib.py" VERBATIM)
add_custom_target(tbb_implib DEPENDS "${CMAKE_BINARY_DIR}/tbb.lib")

# Both originals: base 0x400000, no ASLR, no .reloc. The recompiled exes keep .reloc (/FIXED:NO):
# reccmp uses an image's relocation table to tell addresses from constants, and only the original
# has to go without. /OPT:NOREF keeps every function so reccmp can score code nothing calls yet;
# /OPT:ICF is the 2016 value (EnableCOMDATFolding), TODO prove from a FOLDED pair; it is why STUB() exists.
option(OGRE_KEEP_UNREFERENCED "Keep unreferenced functions so reccmp can compare them" ON)
# The library TUs reference Roblox code no default-build library defines yet: FLog::* and
# FFlag::MarkRakNetSocketOnDelete (Log.dll), RBX::ProtectedString and MD5Hasher, the RobloxExtraSpace
# ctor/dtor (RBX code compiled into lstate.c in 2013), RBX::DataModel::sendStats, plus RakNet's
# PacketizedTCP/TCPInterface/CSHA1, which PLAYER never linked (/OPT:REF dropped their only callers).
# /FORCE:UNRESOLVED links anyway; each such call then has target 0 and differs in reccmp. Turn it off
# once App, Base and Log define them. (Declared above OGRE_PRIOR_APP: Log.dll reads it too.)
function(ogre_executable NAME OUTPUT_NAME)
    set(objects)
    foreach(lib ${ARGN})
        list(APPEND objects $<TARGET_OBJECTS:${lib}>)
    endforeach()
    add_executable(${NAME} WIN32 ${objects})
    target_link_libraries(${NAME} PRIVATE boost ws2_32)  # boost.dll (PLAYER imports 18 names); RakNet's winsock
    if(OGRE_PRIOR_APP)
        target_link_libraries(${NAME} PRIVATE Log "${CMAKE_BINARY_DIR}/tbb.lib")
        add_dependencies(${NAME} tbb_implib)
    endif()
    set_target_properties(${NAME} PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})
    if(OGRE_KEEP_UNREFERENCED)
        set(opt /OPT:NOREF /OPT:ICF)
    else()
        set(opt /OPT:REF /OPT:ICF)
    endif()
    # /LTCG: the client shells are /GL (see OGRE_WINDOWSCLIENT_FLAGS); naming it avoids link.exe's restart.
    target_link_options(${NAME} PRIVATE /DEBUG /INCREMENTAL:NO /BASE:0x400000 /FIXED:NO /DYNAMICBASE:NO
        ${opt} /LTCG "/MAP:$<TARGET_FILE_DIR:${NAME}>/${OUTPUT_NAME}.map")
    if(OGRE_FORCE_UNRESOLVED)
        target_link_options(${NAME} PRIVATE /FORCE:UNRESOLVED)
    endif()
endfunction()

# Link stubs (tools/gen_link_stubs.py, never hand-edited): one STUB function per unwritten callee with
# a known original address, bound to the caller's decorated name by /alternatename, so call sites pair
# with the original callee. Each exe links only its own target's stubs.
# Link order = the 2013 .text order (each library's assert-string functions, ida/exports/<T>-evidence),
# which also decides which object's copy of an inline function the linker keeps (the first): PLAYER's
# RbxOgre copies of App headers carry its Client\App\Include spelling. STUDIO's order differs.
# boost_static goes last, as a prebuilt Contrib2 .lib would: PLAYER keeps App's /GS- copy of the
# inline boost::filesystem::path::string 0x00660160 (lost, 0.49, with boost_static before App).
set(OGRE_PLAYER_LIBS player_client RbxOgre Network GfxAdapters App Base AppDraw GfxBase g3d RbxG3D
    boost_static decomp_util)
set(OGRE_STUDIO_LIBS studio_client RbxOgre Network GfxAdapters RbxG3D g3d GfxBase App Base AppDraw
    boost_static decomp_util)
foreach(t player studio)
    file(GLOB stubs CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/Client/_generated/${t}/*.cpp")
    if(stubs AND OGRE_PRIOR_APP)
        ogre_library(${t}_stubs SOURCES ${stubs} FLAGS ${OGRE_APP_FLAGS})
        string(TOUPPER ${t} T)
        list(APPEND OGRE_${T}_LIBS ${t}_stubs)
    endif()
endforeach()

# The exes are defined in cmake/link/ only so their link lines go through a response file there
# (CMAKE_NINJA_FORCE_RESPONSE_FILE is per directory): with the App tree a link line is ~40 KB, past
# Windows' 32,767-character limit, and wine's link.exe then exits 1 with no output. Compiles stay
# in this directory, without response files (winewrap.py's @file translation cannot handle
# -I"path with spaces"). The exes, PDBs and maps still land in the build root.
add_subdirectory(cmake/link)

reccmp_add_target(player ID PLAYER)
reccmp_add_target(studio ID STUDIO)
reccmp_configure()
