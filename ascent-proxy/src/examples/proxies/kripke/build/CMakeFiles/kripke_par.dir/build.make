# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.21

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/kyan2/ascent/src/examples/proxies/kripke

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kyan2/ascent/src/examples/proxies/kripke/build

# Include any dependencies generated for this target.
include CMakeFiles/kripke_par.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/kripke_par.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/kripke_par.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/kripke_par.dir/flags.make

CMakeFiles/kripke_par.dir/Kripke/Directions.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Directions.o: ../Kripke/Directions.cpp
CMakeFiles/kripke_par.dir/Kripke/Directions.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Directions.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Directions.o -MF CMakeFiles/kripke_par.dir/Kripke/Directions.o.d -o CMakeFiles/kripke_par.dir/Kripke/Directions.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Directions.cpp

CMakeFiles/kripke_par.dir/Kripke/Directions.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Directions.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Directions.cpp > CMakeFiles/kripke_par.dir/Kripke/Directions.i

CMakeFiles/kripke_par.dir/Kripke/Directions.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Directions.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Directions.cpp -o CMakeFiles/kripke_par.dir/Kripke/Directions.s

CMakeFiles/kripke_par.dir/Kripke/Grid.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Grid.o: ../Kripke/Grid.cpp
CMakeFiles/kripke_par.dir/Kripke/Grid.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Grid.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Grid.o -MF CMakeFiles/kripke_par.dir/Kripke/Grid.o.d -o CMakeFiles/kripke_par.dir/Kripke/Grid.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Grid.cpp

CMakeFiles/kripke_par.dir/Kripke/Grid.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Grid.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Grid.cpp > CMakeFiles/kripke_par.dir/Kripke/Grid.i

CMakeFiles/kripke_par.dir/Kripke/Grid.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Grid.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Grid.cpp -o CMakeFiles/kripke_par.dir/Kripke/Grid.s

CMakeFiles/kripke_par.dir/Kripke/Kernel.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Kernel.o: ../Kripke/Kernel.cpp
CMakeFiles/kripke_par.dir/Kripke/Kernel.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Kernel.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Kernel.o -MF CMakeFiles/kripke_par.dir/Kripke/Kernel.o.d -o CMakeFiles/kripke_par.dir/Kripke/Kernel.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel.cpp

CMakeFiles/kripke_par.dir/Kripke/Kernel.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Kernel.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel.cpp > CMakeFiles/kripke_par.dir/Kripke/Kernel.i

CMakeFiles/kripke_par.dir/Kripke/Kernel.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Kernel.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel.cpp -o CMakeFiles/kripke_par.dir/Kripke/Kernel.s

CMakeFiles/kripke_par.dir/Kripke/Layout.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Layout.o: ../Kripke/Layout.cpp
CMakeFiles/kripke_par.dir/Kripke/Layout.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Layout.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Layout.o -MF CMakeFiles/kripke_par.dir/Kripke/Layout.o.d -o CMakeFiles/kripke_par.dir/Kripke/Layout.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Layout.cpp

CMakeFiles/kripke_par.dir/Kripke/Layout.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Layout.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Layout.cpp > CMakeFiles/kripke_par.dir/Kripke/Layout.i

CMakeFiles/kripke_par.dir/Kripke/Layout.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Layout.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Layout.cpp -o CMakeFiles/kripke_par.dir/Kripke/Layout.s

CMakeFiles/kripke_par.dir/Kripke/Subdomain.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Subdomain.o: ../Kripke/Subdomain.cpp
CMakeFiles/kripke_par.dir/Kripke/Subdomain.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Subdomain.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Subdomain.o -MF CMakeFiles/kripke_par.dir/Kripke/Subdomain.o.d -o CMakeFiles/kripke_par.dir/Kripke/Subdomain.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Subdomain.cpp

CMakeFiles/kripke_par.dir/Kripke/Subdomain.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Subdomain.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Subdomain.cpp > CMakeFiles/kripke_par.dir/Kripke/Subdomain.i

CMakeFiles/kripke_par.dir/Kripke/Subdomain.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Subdomain.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Subdomain.cpp -o CMakeFiles/kripke_par.dir/Kripke/Subdomain.s

CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.o: ../Kripke/Sweep_Solver.cpp
CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.o -MF CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.o.d -o CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Sweep_Solver.cpp

CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Sweep_Solver.cpp > CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.i

CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Sweep_Solver.cpp -o CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.s

CMakeFiles/kripke_par.dir/Kripke/ParallelComm.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/ParallelComm.o: ../Kripke/ParallelComm.cpp
CMakeFiles/kripke_par.dir/Kripke/ParallelComm.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/ParallelComm.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/ParallelComm.o -MF CMakeFiles/kripke_par.dir/Kripke/ParallelComm.o.d -o CMakeFiles/kripke_par.dir/Kripke/ParallelComm.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/ParallelComm.cpp

CMakeFiles/kripke_par.dir/Kripke/ParallelComm.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/ParallelComm.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/ParallelComm.cpp > CMakeFiles/kripke_par.dir/Kripke/ParallelComm.i

CMakeFiles/kripke_par.dir/Kripke/ParallelComm.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/ParallelComm.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/ParallelComm.cpp -o CMakeFiles/kripke_par.dir/Kripke/ParallelComm.s

CMakeFiles/kripke_par.dir/Kripke/Timing.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Timing.o: ../Kripke/Timing.cpp
CMakeFiles/kripke_par.dir/Kripke/Timing.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Timing.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Timing.o -MF CMakeFiles/kripke_par.dir/Kripke/Timing.o.d -o CMakeFiles/kripke_par.dir/Kripke/Timing.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Timing.cpp

CMakeFiles/kripke_par.dir/Kripke/Timing.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Timing.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Timing.cpp > CMakeFiles/kripke_par.dir/Kripke/Timing.i

CMakeFiles/kripke_par.dir/Kripke/Timing.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Timing.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Timing.cpp -o CMakeFiles/kripke_par.dir/Kripke/Timing.s

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.o: ../Kripke/Kernel/Kernel_3d_GDZ.cpp
CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.o -MF CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.o.d -o CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_GDZ.cpp

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_GDZ.cpp > CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.i

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_GDZ.cpp -o CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.s

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.o: ../Kripke/Kernel/Kernel_3d_DGZ.cpp
CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.o -MF CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.o.d -o CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_DGZ.cpp

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_DGZ.cpp > CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.i

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_DGZ.cpp -o CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.s

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.o: ../Kripke/Kernel/Kernel_3d_ZDG.cpp
CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.o -MF CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.o.d -o CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_ZDG.cpp

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_ZDG.cpp > CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.i

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_ZDG.cpp -o CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.s

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.o: ../Kripke/Kernel/Kernel_3d_DZG.cpp
CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.o -MF CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.o.d -o CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_DZG.cpp

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_DZG.cpp > CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.i

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_DZG.cpp -o CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.s

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.o: ../Kripke/Kernel/Kernel_3d_ZGD.cpp
CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.o -MF CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.o.d -o CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_ZGD.cpp

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_ZGD.cpp > CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.i

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_ZGD.cpp -o CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.s

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.o: ../Kripke/Kernel/Kernel_3d_GZD.cpp
CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.o -MF CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.o.d -o CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_GZD.cpp

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_GZD.cpp > CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.i

CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/Kernel/Kernel_3d_GZD.cpp -o CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.s

CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.o: ../Kripke/ParallelComm/BlockJacobiComm.cpp
CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_15) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.o -MF CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.o.d -o CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/ParallelComm/BlockJacobiComm.cpp

CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/ParallelComm/BlockJacobiComm.cpp > CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.i

CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/ParallelComm/BlockJacobiComm.cpp -o CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.s

CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.o: ../Kripke/ParallelComm/SweepComm.cpp
CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_16) "Building CXX object CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.o -MF CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.o.d -o CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.o -c /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/ParallelComm/SweepComm.cpp

CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/ParallelComm/SweepComm.cpp > CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.i

CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/Kripke/ParallelComm/SweepComm.cpp -o CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.s

CMakeFiles/kripke_par.dir/tools/kripke.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/tools/kripke.o: ../tools/kripke.cpp
CMakeFiles/kripke_par.dir/tools/kripke.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_17) "Building CXX object CMakeFiles/kripke_par.dir/tools/kripke.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/tools/kripke.o -MF CMakeFiles/kripke_par.dir/tools/kripke.o.d -o CMakeFiles/kripke_par.dir/tools/kripke.o -c /home/kyan2/ascent/src/examples/proxies/kripke/tools/kripke.cpp

CMakeFiles/kripke_par.dir/tools/kripke.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/tools/kripke.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/tools/kripke.cpp > CMakeFiles/kripke_par.dir/tools/kripke.i

CMakeFiles/kripke_par.dir/tools/kripke.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/tools/kripke.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/tools/kripke.cpp -o CMakeFiles/kripke_par.dir/tools/kripke.s

CMakeFiles/kripke_par.dir/tools/testKernels.o: CMakeFiles/kripke_par.dir/flags.make
CMakeFiles/kripke_par.dir/tools/testKernels.o: ../tools/testKernels.cpp
CMakeFiles/kripke_par.dir/tools/testKernels.o: CMakeFiles/kripke_par.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_18) "Building CXX object CMakeFiles/kripke_par.dir/tools/testKernels.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kripke_par.dir/tools/testKernels.o -MF CMakeFiles/kripke_par.dir/tools/testKernels.o.d -o CMakeFiles/kripke_par.dir/tools/testKernels.o -c /home/kyan2/ascent/src/examples/proxies/kripke/tools/testKernels.cpp

CMakeFiles/kripke_par.dir/tools/testKernels.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kripke_par.dir/tools/testKernels.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kyan2/ascent/src/examples/proxies/kripke/tools/testKernels.cpp > CMakeFiles/kripke_par.dir/tools/testKernels.i

CMakeFiles/kripke_par.dir/tools/testKernels.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kripke_par.dir/tools/testKernels.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kyan2/ascent/src/examples/proxies/kripke/tools/testKernels.cpp -o CMakeFiles/kripke_par.dir/tools/testKernels.s

# Object files for target kripke_par
kripke_par_OBJECTS = \
"CMakeFiles/kripke_par.dir/Kripke/Directions.o" \
"CMakeFiles/kripke_par.dir/Kripke/Grid.o" \
"CMakeFiles/kripke_par.dir/Kripke/Kernel.o" \
"CMakeFiles/kripke_par.dir/Kripke/Layout.o" \
"CMakeFiles/kripke_par.dir/Kripke/Subdomain.o" \
"CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.o" \
"CMakeFiles/kripke_par.dir/Kripke/ParallelComm.o" \
"CMakeFiles/kripke_par.dir/Kripke/Timing.o" \
"CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.o" \
"CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.o" \
"CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.o" \
"CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.o" \
"CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.o" \
"CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.o" \
"CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.o" \
"CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.o" \
"CMakeFiles/kripke_par.dir/tools/kripke.o" \
"CMakeFiles/kripke_par.dir/tools/testKernels.o"

# External object files for target kripke_par
kripke_par_EXTERNAL_OBJECTS =

kripke_par: CMakeFiles/kripke_par.dir/Kripke/Directions.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/Grid.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/Kernel.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/Layout.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/Subdomain.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/Sweep_Solver.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/ParallelComm.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/Timing.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GDZ.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DGZ.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZDG.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_DZG.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_ZGD.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/Kernel/Kernel_3d_GZD.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/ParallelComm/BlockJacobiComm.o
kripke_par: CMakeFiles/kripke_par.dir/Kripke/ParallelComm/SweepComm.o
kripke_par: CMakeFiles/kripke_par.dir/tools/kripke.o
kripke_par: CMakeFiles/kripke_par.dir/tools/testKernels.o
kripke_par: CMakeFiles/kripke_par.dir/build.make
kripke_par: CMakeFiles/kripke_par.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_19) "Linking CXX executable kripke_par"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/kripke_par.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/kripke_par.dir/build: kripke_par
.PHONY : CMakeFiles/kripke_par.dir/build

CMakeFiles/kripke_par.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/kripke_par.dir/cmake_clean.cmake
.PHONY : CMakeFiles/kripke_par.dir/clean

CMakeFiles/kripke_par.dir/depend:
	cd /home/kyan2/ascent/src/examples/proxies/kripke/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kyan2/ascent/src/examples/proxies/kripke /home/kyan2/ascent/src/examples/proxies/kripke /home/kyan2/ascent/src/examples/proxies/kripke/build /home/kyan2/ascent/src/examples/proxies/kripke/build /home/kyan2/ascent/src/examples/proxies/kripke/build/CMakeFiles/kripke_par.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/kripke_par.dir/depend

