# reccmp glue: writes <build>/reccmp-build.yml, which tells reccmp where each recompiled image and
# its PDB are. reccmp-project.yml (repo root) and reccmp-user.yml (gitignored) supply the rest.
#
#   reccmp_add_target(<cmake target> ID <RECCMP ID>)   once per scored executable
#   reccmp_configure()                                 once, after every reccmp_add_target
#
# Then run reccmp from the build directory:  reccmp-reccmp --target PLAYER --nolib
function(reccmp_add_target TARGET)
    cmake_parse_arguments(ARGS "" "ID" "" ${ARGN})
    if(NOT ARGS_ID)
        message(FATAL_ERROR "reccmp_add_target(${TARGET}): missing ID")
    endif()
    set_property(GLOBAL APPEND PROPERTY RECCMP_TARGETS "${TARGET}=${ARGS_ID}")
endfunction()

function(reccmp_configure)
    set(yml "project: '${PROJECT_SOURCE_DIR}'\ntargets:\n")
    get_property(targets GLOBAL PROPERTY RECCMP_TARGETS)
    foreach(entry ${targets})
        string(REPLACE "=" ";" pair "${entry}")
        list(GET pair 0 target)
        list(GET pair 1 id)
        string(APPEND yml "  ${id}:\n    path: '$<TARGET_FILE:${target}>'\n    pdb: '$<TARGET_PDB_FILE:${target}>'\n")
    endforeach()
    file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/reccmp-build.yml" CONTENT "${yml}")
endfunction()
