#==============================================================================
# CMake configuration for G4Bench2
#
# NOTE:
# CACHE variables can be changed in CMake CLI with -D option.
#==============================================================================
# installation prefix
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX $ENV{HOME}/work/g4bench/work
      CACHE PATH "Install prefix" FORCE)
endif()

# Geant4 installation path
set(GEANT4_INSTALL /opt/geant4/11.3.0
    CACHE STRING "Geant4 installation path")

# visualization flag
set(ENABLE_VIS FALSE CACHE BOOL "Enable visualization flag")

# Optimizaton / Debug flags
set(OPTIMIZE TRUE CACHE BOOL "Optimizaton flag (O3)")
set(DEBUG FALSE CACHE BOOL "Debug mode")

# Development flag (set false for release)
set(DEVMODE FALSE CACHE BOOL "Development mode")

# spacial flags for developer
option(GEANT4_STATIC "Use static library" OFF)
