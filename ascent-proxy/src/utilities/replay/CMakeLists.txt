###############################################################################
# Copyright (c) Lawrence Livermore National Security, LLC and other Ascent
# Project developers. See top-level LICENSE AND COPYRIGHT files for dates and
# other details. No copyright assignment is required to contribute to Ascent.
###############################################################################

###############################################################################
#
# Replay CMake Build for Ascent
#
###############################################################################

set(REPLAY_SOURCES
    replay.cpp)

set(replay_deps ascent)

if(OPENMP_FOUND)
   list(APPEND deps openmp)
endif()

if (ENABLE_SERIAL)
    blt_add_executable(
        NAME        ascent_replay
        SOURCES     ${REPLAY_SOURCES}
        DEPENDS_ON  ${replay_deps}
        OUTPUT_DIR  ${CMAKE_CURRENT_BINARY_DIR})
    
    # install target for replay serial
    install(TARGETS ascent_replay
            EXPORT  ascent
            LIBRARY DESTINATION utilities/ascent/replay
            ARCHIVE DESTINATION utilities/ascent/replay
            RUNTIME DESTINATION utilities/ascent/replay
    )
endif()

if(MPI_FOUND)

    set(ascent_replay_mpi_deps ascent_mpi mpi)
    if(OPENMP_FOUND)
           list(APPEND ascent_replay_mpi_deps openmp)
    endif()

    blt_add_executable(
        NAME        ascent_replay_mpi
        SOURCES     ${REPLAY_SOURCES}
        DEPENDS_ON  ${ascent_replay_mpi_deps}
        OUTPUT_DIR  ${CMAKE_CURRENT_BINARY_DIR})

    blt_add_target_compile_flags(TO ascent_replay_mpi FLAGS "-DASCENT_REPLAY_MPI=1")

    # install target for replay mpi
    install(TARGETS ascent_replay_mpi
            EXPORT  ascent
            LIBRARY DESTINATION utilities/ascent/replay
            ARCHIVE DESTINATION utilities/ascent/replay
            RUNTIME DESTINATION utilities/ascent/replay
    )


endif()






