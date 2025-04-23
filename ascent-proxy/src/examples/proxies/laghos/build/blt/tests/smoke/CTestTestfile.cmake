# CMake generated Testfile for 
# Source directory: /home/kyan2/ascent/src/blt/tests/smoke
# Build directory: /home/kyan2/ascent/src/examples/proxies/laghos/build/blt/tests/smoke
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(blt_gtest_smoke "/home/kyan2/ascent/src/examples/proxies/laghos/build/tests/blt_gtest_smoke")
set_tests_properties(blt_gtest_smoke PROPERTIES  _BACKTRACE_TRIPLES "/home/kyan2/ascent/src/blt/cmake/BLTMacros.cmake;451;add_test;/home/kyan2/ascent/src/blt/tests/smoke/CMakeLists.txt;20;blt_add_test;/home/kyan2/ascent/src/blt/tests/smoke/CMakeLists.txt;0;")
add_test(blt_mpi_smoke "/usr/bin/mpiexec" "-n" "4" "/home/kyan2/ascent/src/examples/proxies/laghos/build/tests/blt_mpi_smoke")
set_tests_properties(blt_mpi_smoke PROPERTIES  _BACKTRACE_TRIPLES "/home/kyan2/ascent/src/blt/cmake/BLTMacros.cmake;451;add_test;/home/kyan2/ascent/src/blt/tests/smoke/CMakeLists.txt;119;blt_add_test;/home/kyan2/ascent/src/blt/tests/smoke/CMakeLists.txt;0;")
