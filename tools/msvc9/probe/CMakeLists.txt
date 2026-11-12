# Toolchain probe: one OBJECT library and one exe, built with cmake/msvc9-wine.cmake.
#   cmake -G Ninja -S tools/msvc9/probe -B <build> -DCMAKE_TOOLCHAIN_FILE=cmake/msvc9-wine.cmake
cmake_minimum_required(VERSION 3.20)
project(msvc9probe CXX)
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Zi")
add_library(probelib OBJECT probelib.cpp)
add_executable(probe probe.cpp $<TARGET_OBJECTS:probelib>)
target_link_options(probe PRIVATE /DEBUG /FIXED /BASE:0x400000 /OPT:REF /OPT:ICF /INCREMENTAL:NO)
