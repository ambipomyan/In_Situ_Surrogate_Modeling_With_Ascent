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
CMAKE_SOURCE_DIR = /home/kyan2/ascent/src/examples/proxies/laghos

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kyan2/ascent/src/examples/proxies/laghos/build

# Utility rule file for ContinuousUpdate.

# Include any custom commands dependencies for this target.
include blt/thirdparty_builtin/CMakeFiles/ContinuousUpdate.dir/compiler_depend.make

# Include the progress variables for this target.
include blt/thirdparty_builtin/CMakeFiles/ContinuousUpdate.dir/progress.make

blt/thirdparty_builtin/CMakeFiles/ContinuousUpdate:
	cd /home/kyan2/ascent/src/examples/proxies/laghos/build/blt/thirdparty_builtin && /usr/local/bin/ctest -D ContinuousUpdate

ContinuousUpdate: blt/thirdparty_builtin/CMakeFiles/ContinuousUpdate
ContinuousUpdate: blt/thirdparty_builtin/CMakeFiles/ContinuousUpdate.dir/build.make
.PHONY : ContinuousUpdate

# Rule to build all files generated by this target.
blt/thirdparty_builtin/CMakeFiles/ContinuousUpdate.dir/build: ContinuousUpdate
.PHONY : blt/thirdparty_builtin/CMakeFiles/ContinuousUpdate.dir/build

blt/thirdparty_builtin/CMakeFiles/ContinuousUpdate.dir/clean:
	cd /home/kyan2/ascent/src/examples/proxies/laghos/build/blt/thirdparty_builtin && $(CMAKE_COMMAND) -P CMakeFiles/ContinuousUpdate.dir/cmake_clean.cmake
.PHONY : blt/thirdparty_builtin/CMakeFiles/ContinuousUpdate.dir/clean

blt/thirdparty_builtin/CMakeFiles/ContinuousUpdate.dir/depend:
	cd /home/kyan2/ascent/src/examples/proxies/laghos/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kyan2/ascent/src/examples/proxies/laghos /home/kyan2/ascent/src/blt/thirdparty_builtin /home/kyan2/ascent/src/examples/proxies/laghos/build /home/kyan2/ascent/src/examples/proxies/laghos/build/blt/thirdparty_builtin /home/kyan2/ascent/src/examples/proxies/laghos/build/blt/thirdparty_builtin/CMakeFiles/ContinuousUpdate.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : blt/thirdparty_builtin/CMakeFiles/ContinuousUpdate.dir/depend

