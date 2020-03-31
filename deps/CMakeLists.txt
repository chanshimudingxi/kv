cmake_minimum_required(VERSION 2.8)

add_definitions(-ggdb3 -Werror -Wno-builtin-macro-redefined -Wno-deprecated)
add_definitions(-DIM_RUN_MODE)

if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_LESS 2.8.12)
    add_definitions(-std=c++0x)
else()
    add_compile_options(-std=c++0x)
endif()

add_subdirectory( net )
add_subdirectory( data )
add_subdirectory( googletest )
enable_testing()
add_subdirectory( tests )