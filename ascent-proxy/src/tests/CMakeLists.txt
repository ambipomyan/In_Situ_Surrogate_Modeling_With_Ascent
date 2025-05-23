###############################################################################
# Copyright (c) Lawrence Livermore National Security, LLC and other Ascent
# Project developers. See top-level LICENSE AND COPYRIGHT files for dates and
# other details. No copyright assignment is required to contribute to Ascent.
###############################################################################


#define ASCENT_T_SRC_DIR   "@CMAKE_CURRENT_SOURCE_DIR@"
#define ASCENT_T_BIN_DIR   "@CMAKE_CURRENT_BINARY_DIR@"
#define ASCENT_T_DATA_DIR  "@CMAKE_CURRENT_SOURCE_DIR@/data/"



convert_to_native_escaped_file_path(${CMAKE_CURRENT_SOURCE_DIR}
                                    ASCENT_T_SRC_DIR)

convert_to_native_escaped_file_path(${CMAKE_CURRENT_BINARY_DIR}
                                     ASCENT_T_BIN_DIR)

convert_to_native_escaped_file_path(${CMAKE_CURRENT_SOURCE_DIR}/data/
                                    ASCENT_T_DATA_DIR)


# this header allows us to easily use the cmake source and binary paths in
# our unit tests
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/t_config.hpp.in"
                "${CMAKE_CURRENT_BINARY_DIR}/t_config.hpp")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# for t_utils.hpp
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#for t_test_utils.hpp
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/vtkh")
################################
# Unit Tests
################################
# add tpl smoke tests, so they run before we run the ascent tests
add_subdirectory("thirdparty")

# add logging tests
add_subdirectory("logging")

# add apcomp tests
if(ENABLE_APCOMP)
    add_subdirectory("apcomp")
endif()

# add dray tests
if(ENABLE_DRAY)
    add_subdirectory("dray")
endif()

# add vtkh tests
if(ENABLE_VTKH)
    add_subdirectory("vtkh")
endif()

# add flow tests
add_subdirectory("flow")

# add ascent tests
add_subdirectory("ascent")

# add ascent performance tests
add_subdirectory("perf")

# add utils tests
add_subdirectory("utilities")


if(PYTHON_FOUND)
# if we have python:
#  add custom command that generates a html img comparison report for our tests
#  run with "make gen_report"
#  creates "_output/tout_img_report.html"
    add_custom_target(gen_report
                      ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gen_img_compare_html_report.py
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Generating html summary of image compare tests (tests/_output/index.html)")
endif()
