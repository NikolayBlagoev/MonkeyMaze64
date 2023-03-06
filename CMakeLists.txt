cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(ComputerGraphics C CXX)

# Set this before including framework such that it knows to use the OpenGL4.5 version of GLAD
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/framework")
	# Create framework library and include CMake scripts (compiler warnings, sanitizers and static analyzers).
	add_subdirectory("framework")
else()
	# During development the framework lives in parent folder.
	add_subdirectory("../../../framework/" "${CMAKE_BINARY_DIR}/framework/")
endif()

# Additional source files
add_library(FinalProject "")
enable_sanitizers(FinalProject)
set_project_warnings(FinalProject)
include(${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt)
target_include_directories(FinalProject PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/")
target_compile_features(FinalProject PUBLIC cxx_std_20)
target_link_libraries(FinalProject PUBLIC CGFramework)

# Main executable config
add_executable(FinalExecutable "src/main.cpp")
enable_sanitizers(FinalExecutable)
set_project_warnings(FinalExecutable)
target_compile_features(FinalExecutable PUBLIC cxx_std_20)
target_link_libraries(FinalExecutable PRIVATE FinalProject)

# Preprocessor definitions for paths
target_compile_definitions(FinalProject PUBLIC "-DRESOURCES_DIR=\"${CMAKE_CURRENT_LIST_DIR}/resources/\"" "-DSHADERS_DIR=\"${CMAKE_CURRENT_LIST_DIR}/shaders/\"")
