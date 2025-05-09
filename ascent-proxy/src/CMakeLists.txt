###############################################################################
# Copyright (c) Lawrence Livermore National Security, LLC and other Ascent
# Project developers. See top-level LICENSE AND COPYRIGHT files for dates and
# other details. No copyright assignment is required to contribute to Ascent.
###############################################################################

# avoid warnings for project commands w/o VERSION
cmake_policy(SET CMP0048 NEW)

################################
# Ascent
################################

project(ascent VERSION "0.9.3")

################################
# Build Options
################################
option(BUILD_SHARED_LIBS  "Build shared libraries"    ON)
option(ENABLE_TESTS       "Build tests"               ON)

option(ENABLE_FORTRAN     "Build Fortran support"     ON)
option(ENABLE_PYTHON      "Build Python Support"      ON)

option(ENABLE_MPI         "Build MPI Support"         ON)
option(ENABLE_SERIAL      "Build Serial (non-MPI) Support" ON)
option(ENABLE_CUDA        "Build CUDA Support"        OFF)
option(ENABLE_OPENMP      "Build OpenMP Support"      OFF)

# builtin libs: dray, apcomp, vtkh
option(ENABLE_DRAY        "Build Devil Ray Support"   OFF)
# Note: ENABLE_DRAY reqs ENABLE_APCOMP
option(ENABLE_APCOMP      "Build AP Compositor"       OFF)
option(ENABLE_VTKH        "Build VTK-h"               OFF)

option(ENABLE_EXAMPLES    "Build Examples"            ON)
option(ENABLE_UTILS       "Build Utilities"           ON)
option(ENABLE_LOGGING     "Enable data logging"       OFF)
option(ENABLE_DOCS        "Build Documentation"       ON)

option(ENABLE_HIDDEN_VISIBILITY "Build with hidden visibility for private symbols" ON)

# DRay Specific Options
option(DRAY_ENABLE_STATS          "Enable stats"         ON)
option(DRAY_USE_DOUBLE_PRECISION  "Build Devil Ray with double precision" OFF)

# VTK-h Specific Options 

option(VTKH_ENABLE_FILTER_CONTOUR_TREE "Build VTK-h contour tree support" ON)

# Cuda Specific Options
option(ENABLE_CUDA_DEBUG_CPU_ONLY "Enable CUDA CPU debugging" OFF)

#####################################################
# Note: Third party libs like MFEM, FIDES, etc are 
#       enabled when you provide MFEM_DIR, etc
#####################################################

if(NOT ENABLE_SERIAL AND NOT ENABLE_MPI)
    message(FATAL_ERROR "No libraries are built. "
            "Please set ENABLE_SERIAL, ENABLE_MPI or both to ON")
endif()

if(ENABLE_DRAY AND NOT ENABLE_APCOMP)
    message(FATAL_ERROR "Devil Ray requires APComp (ENABLE_APCOMP=OFF and ENABLE_DRAY=ON)")
endif()

if (ENABLE_HIP)
    cmake_minimum_required(VERSION 3.21)
else()
    cmake_minimum_required(VERSION 3.14)
endif()

if(ENABLE_CUDA)
    # ask politely a few times for SEPARABLE COMPILATION
    set (CMAKE_CUDA_SEPARABLE_COMPILATION ON CACHE BOOL "" )
    set (CUDA_SEPARABLE_COMPILATION ON CACHE BOOL "" )
endif()

################################
# cmake policy selection
################################
# cmake 3.18+ use CMP0104 OLD to manually specify cuda flags
#https://cmake.org/cmake/help/latest/policy/CMP0104.html
if(POLICY CMP0104)
    cmake_policy(SET CMP0104 OLD)
endif()

# allow find_packages to use ZZZ_ROOT vars
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

# allow FindPythonInterp and FindPythonLibs
# https://cmake.org/cmake/help/latest/policy/CMP0148.html
if(POLICY CMP0148)
    cmake_policy(SET CMP0148 OLD)
endif()


################################
# Populate the cmake gui
################################
set(CONDUIT_DIR "" CACHE PATH "path to conduit installation")

################################
# Invoke CMake Fortran setup
# if ENABLE_FORTRAN == ON
################################
if(ENABLE_FORTRAN)
    enable_language(Fortran)
endif()

################################
# Init BLT
################################
# This also includes 
# our BLT defaults
include(cmake/SetupBLT.cmake)

################################
# Basic CMake Setup
################################
include(cmake/CMakeBasics.cmake)

##################################
# Helpers for  VTKm device symbols
##################################
include(cmake/VTKmDeviceSymbols.cmake)

################################
# Setup Fortran Support
################################
include(cmake/SetupFortran.cmake)

################################
# Setup 3rd Party Libs
################################
include(cmake/Setup3rdParty.cmake)

################################
# Setup tests helpers
################################
include(cmake/SetupTests.cmake)

################################
# Setup project wide includes
################################
include(cmake/SetupIncludes.cmake)

################################
# Add builtin third party libs
################################
add_subdirectory(thirdparty_builtin)

################################
# Add our libs
################################
add_subdirectory(libs)

################################
# Add mini-app examples
################################
if(ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()

################################
# Add our tests
################################
if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()

################################
# Add utilities
################################
if(ENABLE_UTILS)
   add_subdirectory(utilities)
endif()

################################
# Add documentation targets
################################
if(ENABLE_DOCS)
  add_subdirectory(docs)
endif()

################################
# Add our config helpers
################################
add_subdirectory(config)

################################
# Create CMake importable
# exports for all of our targets
################################
if(WIN32)
    install(EXPORT ascent DESTINATION cmake)
else()
    install(EXPORT ascent DESTINATION lib/cmake/${PROJECT_NAME})
endif()
