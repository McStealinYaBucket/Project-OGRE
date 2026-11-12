# CMake toolchain: VC9 SP1 (cl 15.00.30729.01) from toolchain/msvc9 under wine, via the wrappers
# in tools/msvc9 (path translation, INCLUDE/LIB, mspdbsrv start, ntlm_auth stand-in).
#
#   cmake -G Ninja -S <src> -B <build> -DCMAKE_TOOLCHAIN_FILE=<repo>/cmake/msvc9-wine.cmake [-DOGRE_TARGET=STUDIO]
#   cmake --build <build>
#
# Builds Windows x86 binaries. Manifests are not embedded (/MANIFEST:NO): the original exes
# carry theirs as a resource, and CMake's vs_link_exe manifest step does not run on Linux.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 5.1)
set(CMAKE_SYSTEM_PROCESSOR x86)

set(_msvc9 "${CMAKE_CURRENT_LIST_DIR}/../tools/msvc9")
get_filename_component(_msvc9 "${_msvc9}" ABSOLUTE)
set(CMAKE_C_COMPILER "${_msvc9}/cl")
set(CMAKE_CXX_COMPILER "${_msvc9}/cl")
set(CMAKE_ASM_MASM_COMPILER "${_msvc9}/ml")
set(CMAKE_RC_COMPILER "${_msvc9}/rc")
set(CMAKE_LINKER "${_msvc9}/link" CACHE FILEPATH "" FORCE)
set(CMAKE_AR "${_msvc9}/lib" CACHE FILEPATH "" FORCE)
set(CMAKE_MT "${_msvc9}/mt" CACHE FILEPATH "" FORCE)

# Whose 2013 build machine cl sees (tools/msvc9/winewrap.py): sources and headers are compiled from
# C:\BuildAgent\work\<that build's checkout>\Client\... in the 2013 spelling, so assert __FILE__
# strings match. PLAYER and STUDIO had different checkouts, so each target needs its own build directory:
#   build/          -DOGRE_TARGET=PLAYER (default)   score PLAYER here
#   build-studio/   -DOGRE_TARGET=STUDIO             score STUDIO here
set(OGRE_TARGET PLAYER CACHE STRING "2013 build paths to compile with: PLAYER or STUDIO")
set_property(CACHE OGRE_TARGET PROPERTY STRINGS PLAYER STUDIO)
set(CMAKE_C_COMPILER_LAUNCHER env OGRE_TARGET=${OGRE_TARGET})
set(CMAKE_CXX_COMPILER_LAUNCHER env OGRE_TARGET=${OGRE_TARGET})

set(CMAKE_EXE_LINKER_FLAGS_INIT "/MANIFEST:NO")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "/MANIFEST:NO")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "/MANIFEST:NO")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
