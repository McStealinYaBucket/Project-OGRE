# Both exes, linked through response files (see the add_subdirectory comment in the top CMakeLists.txt).
set(CMAKE_NINJA_FORCE_RESPONSE_FILE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
ogre_executable(player RobloxPlayerBeta ${OGRE_PLAYER_LIBS})
ogre_executable(studio RobloxStudioBeta ${OGRE_STUDIO_LIBS})
