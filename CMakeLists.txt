# The first line of any CMake project should be a call to `cmake_minimum_required`, which checks
# that the installed CMake will be able to understand the following CMakeLists, and ensures that
# CMake's behaviour is compatible with the named version. This is a standard CMake command, so more
# information can be found in the CMake docs.

cmake_minimum_required(VERSION 3.24)

# Sets the minimum macOS version
if (APPLE)
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum version of the target platform" FORCE) 
	if(CMAKE_OSX_DEPLOYMENT_TARGET)
		message("The minimum macOS version is set to " $CACHE{CMAKE_OSX_DEPLOYMENT_TARGET}.)
	endif()
endif ()

# The top-level CMakeLists.txt file for a project must contain a literal, direct call to the
# `project()` command. `project()` sets up some helpful variables that describe source/binary
# directories, and the current project version. This is a standard CMake command.

project(Scyclone VERSION 0.0.3)
set (TARGET_NAME ${PROJECT_NAME})

if(APPLE)
	set (FORMATS_TO_BUILD AU VST3 Standalone)
else()
	set (FORMATS_TO_BUILD VST3 Standalone)
endif()

#static linking runtime library in Windows (for onnxruntime)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(modules/JUCE)                    # If you've put JUCE in a subdirectory called JUCE

# If you are building a VST2 or AAX plugin, CMake needs to be told where to find these SDKs on your
# system. This setup should be done before calling `juce_add_plugin`.

# juce_set_vst2_sdk_path(...)
# juce_set_aax_sdk_path(...)

# `juce_add_plugin` adds a static library target with the name passed as the first argument
# (Scyclone here). This target is a normal CMake target, but has a lot of extra properties set
# up by default. As well as this shared code static library, this function adds targets for each of
# the formats specified by the FORMATS arguments. This function accepts many optional arguments.
# Check the readme at `docs/CMake API.md` in the JUCE repo for the full list.

juce_add_plugin(${TARGET_NAME}
		# VERSION ...                               # Set this if the plugin version is different to the project version
		# ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
		# ICON_SMALL ...
		COMPANY_NAME "Torsion Audio"
		# IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
		# NEEDS_MIDI_INPUT TRUE/FALSE               # Does the plugin need midi input?
		# NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
		# IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
		# EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
		# COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
		PLUGIN_MANUFACTURER_CODE TORA               # A four-character manufacturer id with at least one upper-case character
		PLUGIN_CODE SCYC                            # A unique four-character plugin id with exactly one upper-case character
		# GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case

		if(APPLE)
			HARDENED_RUNTIME_ENABLED TRUE
			HARDENED_RUNTIME_OPTIONS "com.apple.security.device.audio-input"
			MICROPHONE_PERMISSION_ENABLED TRUE
			MICROPHONE_PERMISSION_TEXT "Need access to your audio interface"
		endif()

		FORMATS ${FORMATS_TO_BUILD}                    # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
		PRODUCT_NAME "Scyclone"			            # The name of the final executable, which can differ from the target name
		)

# `juce_generate_juce_header` will create a JuceHeader.h for a given target, which will be generated
# into your build tree. This should be included with `#include <JuceHeader.h>`. The include path for
# this header will be automatically added to the target. The main function of the JuceHeader is to
# include all your JUCE module headers; if you're happy to include module headers directly, you
# probably don't need to call this.

juce_generate_juce_header(${TARGET_NAME})

# Sets the cpp language minimum
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

add_subdirectory(modules/RnboExport)

# Add all source files to file list
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h)
# list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_LIST_DIR}/source/ui/CustomComponents/OpenGLBackground/OpenGLUtil/OpenGLUtil.h)

# Add all sources to target
target_sources(${TARGET_NAME} PRIVATE ${SOURCES} ${RNBO_SOURCES})

# Add include directories for all folders in the source
file(GLOB_RECURSE source_dirs LIST_DIRECTORIES true ${CMAKE_CURRENT_LIST_DIR}/source/*)
list(APPEND source_dirs ${CMAKE_CURRENT_LIST_DIR}/source)

foreach (dir ${source_dirs})
    if (IS_DIRECTORY ${dir})
        target_include_directories(${TARGET_NAME} PUBLIC ${dir})
    endif ()
endforeach ()

# Make the folder structure visible in the IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX "source" FILES ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/modules PREFIX "modules" FILES ${RNBO_SOURCES})

# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.

target_compile_definitions(${TARGET_NAME}
		PUBLIC
		# JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
		JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
		JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
		JUCE_VST3_CAN_REPLACE_VST2=0
		JUCE_DISPLAY_SPLASH_SCREEN=1
		DONT_SET_USING_JUCE_NAMESPACE=1
		)

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/modules/onnxruntime/include)

# Add the onnxruntime library
add_library(onnxruntime STATIC IMPORTED)

if (APPLE)
	message(STATUS ${CMAKE_HOST_SYSTEM_PROCESSOR})
	if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
		set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/modules/onnxruntime/lib/onnxruntime-osx-x64.a)
	elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
		set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/modules/onnxruntime/lib/onnxruntime-osx-arm64.a)
	else()
		message(FATAL_ERROR "CMAKE_HOST_SYSTEM_PROCESSOR not defined.")
	endif()
elseif (MSVC)
	set_property(TARGET onnxruntime APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)

	set_target_properties(onnxruntime PROPERTIES
			IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
			IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/modules/onnxruntime/lib/onnxruntime-win-x64.lib"
			)
	set_target_properties(onnxruntime PROPERTIES
			MAP_IMPORTED_CONFIG_DEBUG Release
			MAP_IMPORTED_CONFIG_MINSIZEREL Release
			MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
			)
endif()


# If your target needs extra binary assets, you can add them here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when your target is built.

add_subdirectory(assets)

# `target_link_libraries` links libraries and JUCE modules to other libraries or executables. Here,
# we're linking our executable target to the `juce::juce_audio_utils` module. Inter-module
# dependencies are resolved automatically, so `juce_core`, `juce_events` and so on will also be
# linked automatically. If we'd generated a binary data target above, we would need to link to it
# here too. This is a standard CMake command.

target_link_libraries(${TARGET_NAME}
		PUBLIC
		BinaryData
		juce::juce_audio_utils
		juce::juce_dsp
		juce::juce_opengl
		juce::juce_graphics
		juce::juce_gui_basics
		juce::juce_gui_extra
		onnxruntime

		PUBLIC
		juce::juce_recommended_config_flags
		juce::juce_recommended_lto_flags
		juce::juce_recommended_warning_flags
)

# enable ctest
enable_testing()

# this is a test on application level starting the app and checking if it is running
add_test(NAME RegularUsage COMMAND "Scyclone_artefacts/Debug/Standalone/Scyclone.app/Contents/MacOS/Scyclone")
# we can specify som REGEX to check the output of the application
set_tests_properties(RegularUsage
  PROPERTIES FAIL_REGULAR_EXPRESSION "Custom Slider Component"
  )

# get all test cpp and header files
file(GLOB_RECURSE TestFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/test/*.h")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/test PREFIX "" FILES ${TestFiles})

# This module enables populating content at configure time via any method supported by the ExternalProject module. Whereas ExternalProject_Add() downloads at build time, the FetchContent module makes content available immediately, allowing the configure step to use the content in commands like add_subdirectory(), include() or file() operations.
include(FetchContent)

# Externally provided libraries
# Using zip files instead is faster
FetchContent_Declare(googletest
	URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)

FetchContent_Declare(benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.0)

# This command ensures that each of the named dependencies are made available to the project by the time it returns. If the dependency has already been populated the command does nothing. Otherwise, the command populates the dependency and then calls add_subdirectory() on the result.
FetchContent_MakeAvailable(googletest)

# For benchmark we want to set the BENCMARK_ENABLE_TESTING to OFF therefore we cannot use FetchContent_MakeAvailable()
# Check if population has already been performed
FetchContent_GetProperties(benchmark)
if(NOT benchmark_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(benchmark)

  # Set custom variables, policies, etc.
  set(BENCHMARK_ENABLE_TESTING OFF)

  # Bring the populated content into the build
  add_subdirectory(${benchmark_SOURCE_DIR} ${benchmark_BINARY_DIR})
endif()

# Setup the test executable
add_executable(Test ${TestFiles})
set_property(TARGET Test PROPERTY CXX_STANDARD 20)

# Link the test executable against gtest_main and google benchmark
target_link_libraries(Test PRIVATE gtest_main benchmark::benchmark "${PROJECT_NAME}")

# We can't link again to the shared juce target without ODL violations (https://github.com/sudara/pamplejuce)
# However, we can steal its include dirs and compile definitions to use in Test!
# Get the compile definitions from the main target
get_target_property(compile_defs ${PROJECT_NAME} INTERFACE_COMPILE_DEFINITIONS)
# Get the interface include directories from the main target, these are the include directories that have been added to the main target using target_include_directories with the INTERFACE or PUBLIC option
get_target_property(include_directories ${PROJECT_NAME} INTERFACE_INCLUDE_DIRECTORIES)
# Get the juce include directory (where the JuceHeader.h is located)
get_target_property(juce_include_directory ${PROJECT_NAME} JUCE_GENERATED_SOURCES_DIRECTORY)

# Set the compile definitions for the test executable
target_compile_definitions(Test PRIVATE compile_defs)
# Set all the include directories from the main target and add them to the test executable as PRIVATE includes since INTERFACE will only be added to the interface include directory and therfore not found by the test executable
target_include_directories(Test PRIVATE include_directories)
target_include_directories(Test PRIVATE ${juce_include_directory})

# Make an Xcode Scheme for the test executable so we can run Test in the IDE
set_target_properties(Test PROPERTIES XCODE_GENERATE_SCHEME ON)

# Organize the test source in the test/ folder in the IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/test PREFIX "" FILES ${TestFiles})

# include Loads and runs CMake code from the file given. Loads and runs CMake code from the file given.
include(GoogleTest)
# gtest_discover_tests will register a CTest test for each gtest and run them all in parallel with the rest of the Test.
gtest_discover_tests(Test)

# Color our warnings and errors
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   add_compile_options (-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   add_compile_options (-fcolor-diagnostics)
endif ()