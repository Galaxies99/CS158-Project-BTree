# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.13

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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "F:\Software\CLion 2018.3.4\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "F:\Software\CLion 2018.3.4\bin\cmake\win\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = G:\GalaxiesWork\CS158-Project-BTree\BplusTree

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = G:\GalaxiesWork\CS158-Project-BTree\BplusTree\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/BplusTree.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/BplusTree.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/BplusTree.dir/flags.make

CMakeFiles/BplusTree.dir/main.cpp.obj: CMakeFiles/BplusTree.dir/flags.make
CMakeFiles/BplusTree.dir/main.cpp.obj: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=G:\GalaxiesWork\CS158-Project-BTree\BplusTree\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/BplusTree.dir/main.cpp.obj"
	C:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\BplusTree.dir\main.cpp.obj -c G:\GalaxiesWork\CS158-Project-BTree\BplusTree\main.cpp

CMakeFiles/BplusTree.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/BplusTree.dir/main.cpp.i"
	C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E G:\GalaxiesWork\CS158-Project-BTree\BplusTree\main.cpp > CMakeFiles\BplusTree.dir\main.cpp.i

CMakeFiles/BplusTree.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/BplusTree.dir/main.cpp.s"
	C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S G:\GalaxiesWork\CS158-Project-BTree\BplusTree\main.cpp -o CMakeFiles\BplusTree.dir\main.cpp.s

# Object files for target BplusTree
BplusTree_OBJECTS = \
"CMakeFiles/BplusTree.dir/main.cpp.obj"

# External object files for target BplusTree
BplusTree_EXTERNAL_OBJECTS =

BplusTree.exe: CMakeFiles/BplusTree.dir/main.cpp.obj
BplusTree.exe: CMakeFiles/BplusTree.dir/build.make
BplusTree.exe: CMakeFiles/BplusTree.dir/linklibs.rsp
BplusTree.exe: CMakeFiles/BplusTree.dir/objects1.rsp
BplusTree.exe: CMakeFiles/BplusTree.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=G:\GalaxiesWork\CS158-Project-BTree\BplusTree\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable BplusTree.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\BplusTree.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/BplusTree.dir/build: BplusTree.exe

.PHONY : CMakeFiles/BplusTree.dir/build

CMakeFiles/BplusTree.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\BplusTree.dir\cmake_clean.cmake
.PHONY : CMakeFiles/BplusTree.dir/clean

CMakeFiles/BplusTree.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" G:\GalaxiesWork\CS158-Project-BTree\BplusTree G:\GalaxiesWork\CS158-Project-BTree\BplusTree G:\GalaxiesWork\CS158-Project-BTree\BplusTree\cmake-build-debug G:\GalaxiesWork\CS158-Project-BTree\BplusTree\cmake-build-debug G:\GalaxiesWork\CS158-Project-BTree\BplusTree\cmake-build-debug\CMakeFiles\BplusTree.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/BplusTree.dir/depend
