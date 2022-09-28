#
#  CMakeLists.txt for buidlig G4Bench
#
#  Copyright 2022 Koichi Murakami
#
#  ***************************
#  !! DONOT EDIT THIS FILE  !!
#  ***************************
#==============================================================================
cmake_minimum_required(VERSION 3.16)

#==============================================================================
project(G4Bench2 CXX)
#==============================================================================

include(config.cmake)

# version.h generation (true for release)
if(DEVMODE)
  option(SKIP_VERSION "overwrite version.h" OFF)
else()
  option(SKIP_VERSION "overwrite version.h" ON)
endif()

# cmake modules
set(CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/cmake/Modules
    ${CMAKE_MODULE_PATH})

# find packages
find_package(Geant4 REQUIRED)
find_package(OpenGL)

# c++17
add_library(global_cflags INTERFACE)
target_compile_features(global_cflags INTERFACE cxx_std_17)
set(CMAKE_CXX_EXTENSIONS OFF)

# build options
if(NOT CMAKE_BUILD_TYPE)
  if(DEBUG)
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
  elseif(OPTIMIZE)
    set(CMAKE_BUILD_TYPE "Release")
  else()
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
  endif()
endif()

# Geant4 visualization
if(NOT GEANT4_VIS)
  set (ENABLE_VIS FALSE)
endif()

# rpath setting
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_EXE_LINKER_FLAGS "-Wl,--disable-new-dtags")
endif()

#------------------------------------------------------------------------------
# parameters for building
message(STATUS "--------------------------------------------------------")
message(STATUS "Parameters for building")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "GEANT4_LIBRARY_DIR: ${GEANT4_LIBRARY_DIR}")
message(STATUS "ENABLE_VIS: ${ENABLE_VIS}")
message(STATUS "GEANT4_STATIC : ${GEANT4_STATIC}")
message(STATUS "--------------------------------------------------------")

#------------------------------------------------------------------------------
# version.h (#.##d# + rev# / dev flag d/a/b/rc)
if(NOT SKIP_VERSION)
set(G4BENCH_VERSION_MAJOR 2)
set(G4BENCH_VERSION_MINOR 0.0)
execute_process (COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE
                 G4BENCH_VERSION_BUILD OUTPUT_STRIP_TRAILING_WHITESPACE)
configure_file(
  "${PROJECT_SOURCE_DIR}/cmake/version.h.in"
  "${PROJECT_SOURCE_DIR}/version.h"
)
endif()

#------------------------------------------------------------------------------
# adding sub directories to the project
add_subdirectory(ecal)
add_subdirectory(hcal)
add_subdirectory(vgeo)
add_subdirectory(bench)
