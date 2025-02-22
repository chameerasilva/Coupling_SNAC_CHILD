# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_COMMAND = /opt/local/bin/cmake

# The command to remove a file.
RM = /opt/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /opt/local/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl

# Include any dependencies generated for this target.
include bmi/CMakeFiles/bmi.dir/depend.make

# Include the progress variables for this target.
include bmi/CMakeFiles/bmi.dir/progress.make

# Include the compile flags for this target's objects.
include bmi/CMakeFiles/bmi.dir/flags.make

bmi/CMakeFiles/bmi.dir/bmi_impl.c.o: bmi/CMakeFiles/bmi.dir/flags.make
bmi/CMakeFiles/bmi.dir/bmi_impl.c.o: bmi/bmi_impl.c
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object bmi/CMakeFiles/bmi.dir/bmi_impl.c.o"
	cd /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl/bmi && /opt/local/bin/mpicc-mpich-mp  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/bmi.dir/bmi_impl.c.o   -c /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl/bmi/bmi_impl.c

bmi/CMakeFiles/bmi.dir/bmi_impl.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/bmi.dir/bmi_impl.c.i"
	cd /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl/bmi && /opt/local/bin/mpicc-mpich-mp  $(C_DEFINES) $(C_FLAGS) -E /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl/bmi/bmi_impl.c > CMakeFiles/bmi.dir/bmi_impl.c.i

bmi/CMakeFiles/bmi.dir/bmi_impl.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/bmi.dir/bmi_impl.c.s"
	cd /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl/bmi && /opt/local/bin/mpicc-mpich-mp  $(C_DEFINES) $(C_FLAGS) -S /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl/bmi/bmi_impl.c -o CMakeFiles/bmi.dir/bmi_impl.c.s

bmi/CMakeFiles/bmi.dir/bmi_impl.c.o.requires:
.PHONY : bmi/CMakeFiles/bmi.dir/bmi_impl.c.o.requires

bmi/CMakeFiles/bmi.dir/bmi_impl.c.o.provides: bmi/CMakeFiles/bmi.dir/bmi_impl.c.o.requires
	$(MAKE) -f bmi/CMakeFiles/bmi.dir/build.make bmi/CMakeFiles/bmi.dir/bmi_impl.c.o.provides.build
.PHONY : bmi/CMakeFiles/bmi.dir/bmi_impl.c.o.provides

bmi/CMakeFiles/bmi.dir/bmi_impl.c.o.provides.build: bmi/CMakeFiles/bmi.dir/bmi_impl.c.o

# Object files for target bmi
bmi_OBJECTS = \
"CMakeFiles/bmi.dir/bmi_impl.c.o"

# External object files for target bmi
bmi_EXTERNAL_OBJECTS =

bmi/libbmi.dylib: bmi/CMakeFiles/bmi.dir/bmi_impl.c.o
bmi/libbmi.dylib: bmi/CMakeFiles/bmi.dir/build.make
bmi/libbmi.dylib: bmi/CMakeFiles/bmi.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C shared library libbmi.dylib"
	cd /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl/bmi && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bmi.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bmi/CMakeFiles/bmi.dir/build: bmi/libbmi.dylib
.PHONY : bmi/CMakeFiles/bmi.dir/build

bmi/CMakeFiles/bmi.dir/requires: bmi/CMakeFiles/bmi.dir/bmi_impl.c.o.requires
.PHONY : bmi/CMakeFiles/bmi.dir/requires

bmi/CMakeFiles/bmi.dir/clean:
	cd /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl/bmi && $(CMAKE_COMMAND) -P CMakeFiles/bmi.dir/cmake_clean.cmake
.PHONY : bmi/CMakeFiles/bmi.dir/clean

bmi/CMakeFiles/bmi.dir/depend:
	cd /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl/bmi /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl/bmi /Users/eunseo/projects/Coupling_SNAC_CHILD/bmi_impl/bmi/CMakeFiles/bmi.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bmi/CMakeFiles/bmi.dir/depend

