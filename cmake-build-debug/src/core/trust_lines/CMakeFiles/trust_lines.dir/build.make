# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/denis/Downloads/clion-2016.3.2/bin/cmake/bin/cmake

# The command to remove a file.
RM = /home/denis/Downloads/clion-2016.3.2/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/denis/Projects/client

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/denis/Projects/client/cmake-build-debug

# Include any dependencies generated for this target.
include src/core/trust_lines/CMakeFiles/trust_lines.dir/depend.make

# Include the progress variables for this target.
include src/core/trust_lines/CMakeFiles/trust_lines.dir/progress.make

# Include the compile flags for this target's objects.
include src/core/trust_lines/CMakeFiles/trust_lines.dir/flags.make

src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o: src/core/trust_lines/CMakeFiles/trust_lines.dir/flags.make
src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o: ../src/core/trust_lines/manager/TrustLinesManager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/trust_lines && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o -c /home/denis/Projects/client/src/core/trust_lines/manager/TrustLinesManager.cpp

src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.i"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/denis/Projects/client/src/core/trust_lines/manager/TrustLinesManager.cpp > CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.i

src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.s"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/denis/Projects/client/src/core/trust_lines/manager/TrustLinesManager.cpp -o CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.s

src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o.requires:

.PHONY : src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o.requires

src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o.provides: src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o.requires
	$(MAKE) -f src/core/trust_lines/CMakeFiles/trust_lines.dir/build.make src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o.provides.build
.PHONY : src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o.provides

src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o.provides.build: src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o


src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o: src/core/trust_lines/CMakeFiles/trust_lines.dir/flags.make
src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o: ../src/core/trust_lines/TrustLine.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/trust_lines && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/trust_lines.dir/TrustLine.cpp.o -c /home/denis/Projects/client/src/core/trust_lines/TrustLine.cpp

src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/trust_lines.dir/TrustLine.cpp.i"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/denis/Projects/client/src/core/trust_lines/TrustLine.cpp > CMakeFiles/trust_lines.dir/TrustLine.cpp.i

src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/trust_lines.dir/TrustLine.cpp.s"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/trust_lines && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/denis/Projects/client/src/core/trust_lines/TrustLine.cpp -o CMakeFiles/trust_lines.dir/TrustLine.cpp.s

src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o.requires:

.PHONY : src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o.requires

src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o.provides: src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o.requires
	$(MAKE) -f src/core/trust_lines/CMakeFiles/trust_lines.dir/build.make src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o.provides.build
.PHONY : src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o.provides

src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o.provides.build: src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o


# Object files for target trust_lines
trust_lines_OBJECTS = \
"CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o" \
"CMakeFiles/trust_lines.dir/TrustLine.cpp.o"

# External object files for target trust_lines
trust_lines_EXTERNAL_OBJECTS =

src/core/trust_lines/libtrust_lines.a: src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o
src/core/trust_lines/libtrust_lines.a: src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o
src/core/trust_lines/libtrust_lines.a: src/core/trust_lines/CMakeFiles/trust_lines.dir/build.make
src/core/trust_lines/libtrust_lines.a: src/core/trust_lines/CMakeFiles/trust_lines.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/denis/Projects/client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libtrust_lines.a"
	cd /home/denis/Projects/client/cmake-build-debug/src/core/trust_lines && $(CMAKE_COMMAND) -P CMakeFiles/trust_lines.dir/cmake_clean_target.cmake
	cd /home/denis/Projects/client/cmake-build-debug/src/core/trust_lines && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/trust_lines.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/core/trust_lines/CMakeFiles/trust_lines.dir/build: src/core/trust_lines/libtrust_lines.a

.PHONY : src/core/trust_lines/CMakeFiles/trust_lines.dir/build

src/core/trust_lines/CMakeFiles/trust_lines.dir/requires: src/core/trust_lines/CMakeFiles/trust_lines.dir/manager/TrustLinesManager.cpp.o.requires
src/core/trust_lines/CMakeFiles/trust_lines.dir/requires: src/core/trust_lines/CMakeFiles/trust_lines.dir/TrustLine.cpp.o.requires

.PHONY : src/core/trust_lines/CMakeFiles/trust_lines.dir/requires

src/core/trust_lines/CMakeFiles/trust_lines.dir/clean:
	cd /home/denis/Projects/client/cmake-build-debug/src/core/trust_lines && $(CMAKE_COMMAND) -P CMakeFiles/trust_lines.dir/cmake_clean.cmake
.PHONY : src/core/trust_lines/CMakeFiles/trust_lines.dir/clean

src/core/trust_lines/CMakeFiles/trust_lines.dir/depend:
	cd /home/denis/Projects/client/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/denis/Projects/client /home/denis/Projects/client/src/core/trust_lines /home/denis/Projects/client/cmake-build-debug /home/denis/Projects/client/cmake-build-debug/src/core/trust_lines /home/denis/Projects/client/cmake-build-debug/src/core/trust_lines/CMakeFiles/trust_lines.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/core/trust_lines/CMakeFiles/trust_lines.dir/depend

