###############################################################################
# Copyright (c) Lawrence Livermore National Security, LLC and other Ascent
# Project developers. See top-level LICENSE AND COPYRIGHT files for dates and
# other details. No copyright assignment is required to contribute to Ascent.
###############################################################################


###############################################################################
#
# Kripke (steering) CMake Build for Ascent
#
###############################################################################


set(KRIPKE_STEERING_SOURCES
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
    if(OPENMP_FOUND)
       set(kripke_steering_openmp_flags "-DKRIPKE_USE_OPENMP")
    else()
       set(kripke_steering_openmp_flags "")
    endif()

    include_directories(.)
    include_directories("tools")


    set(kripke_steering_par_deps ascent_mpi mpi)
    if(OPENMP_FOUND)
           list(APPEND kripke_steering_par_deps openmp)
    endif()

    blt_add_executable(
        NAME        kripke_steering_par
        SOURCES     ${KRIPKE_STEERING_SOURCES}
        DEPENDS_ON  ${kripke_steering_par_deps}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

    blt_add_target_compile_flags(TO kripke_steering_par FLAGS "-D PARALLEL ${kripke_openmp_flags}")

    # install target for kripke_steering mpi
    install(TARGETS kripke_steering_par
            EXPORT  ascent
            LIBRARY DESTINATION examples/ascent/proxies/kripke-steering
            ARCHIVE DESTINATION examples/ascent/proxies/kripke-steering
            RUNTIME DESTINATION examples/ascent/proxies/kripke-steering
    )

    install(FILES ASCENT_README.md
            DESTINATION examples/ascent/proxies/kripke-steering)

endif()






