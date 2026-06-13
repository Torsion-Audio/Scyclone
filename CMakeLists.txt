cmake_minimum_required(VERSION 3.24)

# The top-level CMakeLists.txt file for a project must contain a literal, direct call to the `project()` command.
# `project()` sets up some helpful variables that describe source/binary directories, and the current project version.
project(Scyclone VERSION 0.0.3)
set (TARGET_NAME ${PROJECT_NAME})

include(cmake/configure_platform.cmake)

# Add JUCE module
add_subdirectory(modules/JUCE)
# JUCE plugin setup
include(cmake/juce_plugin_setup.cmake)
include(cmake/generate_build_info.cmake)


# ONNX Runtime and libsamplerate
include(cmake/setup_onnx_runtime.cmake)
include(cmake/setup_libsamplerate.cmake)

# Tests and Benchmarks
include(cmake/setup_tests_and_benchmarks.cmake)

# JUCE compile definitions and linking
include(cmake/juce_compile_config.cmake)

# Ad-hoc sign plugin bundles after build (macOS dev convenience)
include(cmake/sign_plugin.cmake)

# Add Installer and RNBO
add_subdirectory(installer)
add_subdirectory(modules/RnboExport)

# Add all source files to file list. Todo RNBO and ONNX still partially in here
include(cmake/add_source_files.cmake)

# If your target needs extra binary assets, you can add them here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when your target is built.
add_subdirectory(assets)

# enable ctest
enable_testing()
# this is a test on application level starting the app and checking if it is running
# add_test(NAME RegularUsage COMMAND "Scyclone_artefacts/Debug/Standalone/Scyclone.app/Contents/MacOS/Scyclone")
# we can specify som REGEX to check the output of the application
# set_tests_properties(RegularUsage
#   PROPERTIES FAIL_REGULAR_EXPRESSION "Custom Slider Component"
#   )


# Color our warnings and errors
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_compile_options (-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	add_compile_options (-fcolor-diagnostics)
endif()
