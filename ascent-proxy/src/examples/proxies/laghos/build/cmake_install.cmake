# Install script for directory: /home/kyan2/ascent/src/examples/proxies/laghos

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/examples/ascent/proxies/laghos/laghos_mpi" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/examples/ascent/proxies/laghos/laghos_mpi")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/examples/ascent/proxies/laghos/laghos_mpi"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/examples/ascent/proxies/laghos" TYPE EXECUTABLE FILES "/home/kyan2/ascent/src/examples/proxies/laghos/build/laghos_mpi")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/examples/ascent/proxies/laghos/laghos_mpi" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/examples/ascent/proxies/laghos/laghos_mpi")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/examples/ascent/proxies/laghos/laghos_mpi"
         OLD_RPATH "/usr/lib/x86_64-linux-gnu/openmpi/lib:/home/kyan2/ascent/src/examples/proxies/laghos/mfem:/home/kyan2/ascent/src/examples/proxies/laghos/hypre/src/hypre/lib:/home/kyan2/ascent/src/examples/proxies/laghos/METIS/build/install/lib:/home/kyan2/ascent/src/examples/proxies/laghos/GKlib/build/Linux-x86_64/install/lib:/home/kyan2/ascent/install/ascent-checkout/lib:/home/kyan2/ascent/install/conduit-v0.9.2/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/examples/ascent/proxies/laghos/laghos_mpi")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/examples/ascent/proxies/laghos" TYPE DIRECTORY FILES "/home/kyan2/ascent/src/examples/proxies/laghos/data")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/examples/ascent/proxies/laghos" TYPE FILE FILES
    "/home/kyan2/ascent/src/examples/proxies/laghos/run_taylor_green_2d.sh"
    "/home/kyan2/ascent/src/examples/proxies/laghos/run_taylor_green_3d.sh"
    "/home/kyan2/ascent/src/examples/proxies/laghos/run_tripple_point_2d.sh"
    "/home/kyan2/ascent/src/examples/proxies/laghos/run_tripple_point_3d.sh"
    "/home/kyan2/ascent/src/examples/proxies/laghos/run_sedov_2d.sh"
    "/home/kyan2/ascent/src/examples/proxies/laghos/run_sedov_3d.sh"
    "/home/kyan2/ascent/src/examples/proxies/laghos/ascent_options.yaml"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/kyan2/ascent/src/examples/proxies/laghos/build/blt/thirdparty_builtin/cmake_install.cmake")
  include("/home/kyan2/ascent/src/examples/proxies/laghos/build/blt/tests/smoke/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/kyan2/ascent/src/examples/proxies/laghos/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
