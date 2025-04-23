###############################################################################
# Copyright (c) Lawrence Livermore National Security, LLC and other Ascent
# Project developers. See top-level LICENSE AND COPYRIGHT files for dates and
# other details. No copyright assignment is required to contribute to Ascent.
###############################################################################


###############################################################################
#
# Kripke CMake Build for Ascent
#
###############################################################################

# Include BLT
set(BLT_SOURCE_DIR /home/kyan2/ascent/src/blt)
include(${BLT_SOURCE_DIR}/SetupBLT.cmake)

set(KRIPKE_SOURCES
    Kripke/Directions.cpp
    Kripke/Grid.cpp
    Kripke/Kernel.cpp
    Kripke/Layout.cpp
    Kripke/Subdomain.cpp
    Kripke/Sweep_Solver.cpp
    Kripke/ParallelComm.cpp
    Kripke/Timing.cpp

    Kripke/Kernel/Kernel_3d_GDZ.cpp
    Kripke/Kernel/Kernel_3d_DGZ.cpp
    Kripke/Kernel/Kernel_3d_ZDG.cpp
    Kripke/Kernel/Kernel_3d_DZG.cpp
    Kripke/Kernel/Kernel_3d_ZGD.cpp
    Kripke/Kernel/Kernel_3d_GZD.cpp

    Kripke/ParallelComm/BlockJacobiComm.cpp
    Kripke/ParallelComm/SweepComm.cpp
    tools/kripke.cpp
    tools/testKernels.cpp
)

# kripke reqs mpi
if(MPI_FOUND)
    # copy over example command line
    configure_file(run_kripke_simple_example.sh
                   ${CMAKE_CURRENT_BINARY_DIR}/run_kripke_simple_example.sh
                   COPYONLY)
    configure_file(ascent_actions.yaml ${CMAKE_CURRENT_BINARY_DIR}/ascent_actions.yaml COPYONLY)
    configure_file(ascent_options.yaml ${CMAKE_CURRENT_BINARY_DIR}/ascent_options.yaml COPYONLY)

    if(OPENMP_FOUND)
       set(kripke_openmp_flags "-DKRIPKE_USE_OPENMP")
    else()
       set(kripke_openmp_flags "")
    endif()

    include_directories(.)
    include_directories("tools")


    set(kripke_par_deps ascent_mpi mpi)
    if(OPENMP_FOUND)
           list(APPEND kripke_par_deps openmp)
    endif()

    blt_add_executable(
        NAME        kripke_par
        SOURCES     ${KRIPKE_SOURCES}
        DEPENDS_ON  ${kripke_par_deps}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

    # -I
    target_include_directories(kripke_par PRIVATE
                               /usr/lib/x86_64-linux-gnu/openmpi/include
                               /home/kyan2/ascent/install/conduit-v0.9.2/include/conduit
                               /home/kyan2/ascent/install/ascent-checkout/include/ascent
    )

    # -L
    target_link_directories(kripke_par PRIVATE
                            /home/kyan2/ascent/install/ascent-checkout/lib
                            /home/kyan2/ascent/install/conduit-v0.9.2/lib
                            /usr/lib/x86_64-linux-gnu/openmpi/lib
                            /home/kyan2/ascent/install/vtk-m-v2.1.0/lib
    )

    target_link_libraries(kripke_par PRIVATE ascent_mpi conduit conduit_blueprint mpi_cxx)


    blt_add_target_compile_flags(TO kripke_par FLAGS "-D PARALLEL ${kripke_openmp_flags}")

    # install target for kripke mpi
    install(TARGETS kripke_par
            EXPORT  ascent
            LIBRARY DESTINATION examples/ascent/proxies/kripke
            ARCHIVE DESTINATION examples/ascent/proxies/kripke
            RUNTIME DESTINATION examples/ascent/proxies/kripke
    )

    install(FILES ASCENT_README.md
                  ascent_actions.json
                  ascent_options.json
            DESTINATION examples/ascent/proxies/kripke)

endif()






