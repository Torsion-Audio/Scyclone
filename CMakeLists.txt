# Example Audio Plugin CMakeLists.txt

# To get started on a new plugin, copy this entire folder (containing this file and C++ sources) to
# a convenient location, and then start making modifications.

# The first line of any CMake project should be a call to `cmake_minimum_required`, which checks
# that the installed CMake will be able to understand the following CMakeLists, and ensures that
# CMake's behaviour is compatible with the named version. This is a standard CMake command, so more
# information can be found in the CMake docs.

cmake_minimum_required(VERSION 3.24)

# The top-level CMakeLists.txt file for a project must contain a literal, direct call to the
# `project()` command. `project()` sets up some helpful variables that describe source/binary
# directories, and the current project version. This is a standard CMake command.

project(Scyclone VERSION 0.0.1)
set (target_name ${PROJECT_NAME})

##### Compilation options
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

if(APPLE)
	set (formats AU VST3 Standalone)
else()
	set (formats VST3 Standalone)
endif()

if (MSVC)
	if (EXISTS "C:/Program Files (x86)/Intel/oneAPI/mkl/2023.0.0/lib/intel64")
		link_directories("C:/Program Files (x86)/Intel/oneAPI/mkl/2023.0.0/lib/intel64")
		get_directory_property(_my_link_dirs LINK_DIRECTORIES)
		message(STATUS "_my_link_dirs = ${_my_link_dirs}")
	endif()
endif (MSVC)

#onnxruntime providers
option(ONNXRUNTIME_ROOTDIR "onnxruntime root dir")

if(NOT ONNXRUNTIME_ROOTDIR)
	if (APPLE)
		message(STATUS ${CMAKE_HOST_SYSTEM_PROCESSOR})
		if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
			set (ONNXRUNTIME_ROOTDIR /usr/local/opt/onnxruntime@1.12.1)
		elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
			set (ONNXRUNTIME_ROOTDIR /opt/homebrew/opt/onnxruntime@1.12.1)
		else()
			message(FATAL_ERROR "Check README.md for build instructions on macOS. CMAKE_HOST_SYSTEM_PROCESSOR not defined.")
		endif()
		message(STATUS "Set onnxruntime directory to: ${ONNXRUNTIME_ROOTDIR}")
	else()
		include(cmake/Onnx.cmake)
		set(ONNXRUNTIME_ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime-${LIBONNXRUNTIME_VERSION})
	endif()
endif()

include_directories(
		"${ONNXRUNTIME_ROOTDIR}/include"
		"${ONNXRUNTIME_ROOTDIR}/include/onnxruntime/core/session"
		"${CMAKE_CURRENT_SOURCE_DIR}/modules/RnboExport/rnbo"
		"${CMAKE_CURRENT_SOURCE_DIR}/modules/RnboExport/rnbo/common"
)
link_directories("${ONNXRUNTIME_ROOTDIR}/lib")

# If you've installed JUCE somehow (via a package manager, or directly using the CMake install
# target), you'll need to tell this project that it depends on the installed copy of JUCE. If you've
# included JUCE directly in your source tree (perhaps as a submodule), you'll need to tell CMake to
# include that subdirectory as part of the build.

# find_package(JUCE CONFIG REQUIRED)        # If you've installed JUCE to your system
# or
add_subdirectory(modules/JUCE)                    # If you've put JUCE in a subdirectory called JUCE

# If you are building a VST2 or AAX plugin, CMake needs to be told where to find these SDKs on your
# system. This setup should be done before calling `juce_add_plugin`.

# juce_set_vst2_sdk_path(...)
# juce_set_aax_sdk_path(...)

# `juce_add_plugin` adds a static library target with the name passed as the first argument
# (VAESynth here). This target is a normal CMake target, but has a lot of extra properties set
# up by default. As well as this shared code static library, this function adds targets for each of
# the formats specified by the FORMATS arguments. This function accepts many optional arguments.
# Check the readme at `docs/CMake API.md` in the JUCE repo for the full list.

juce_add_plugin(${target_name}
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
			MICROPHONE_PERMISSION_ENABLED TRUE
			MICROPHONE_PERMISSION_TEXT "Need access to your audio interface"
		endif()

		FORMATS VST3 Standalone                     # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
		PRODUCT_NAME "Scyclone"			            # The name of the final executable, which can differ from the target name
		)


# `juce_generate_juce_header` will create a JuceHeader.h for a given target, which will be generated
# into your build tree. This should be included with `#include <JuceHeader.h>`. The include path for
# this header will be automatically added to the target. The main function of the JuceHeader is to
# include all your JUCE module headers; if you're happy to include module headers directly, you
# probably don't need to call this.

juce_generate_juce_header(${target_name})

# `target_sources` adds source files to a target. We pass the target that needs the sources as the
# first argument, then a visibility parameter for the sources which should normally be PRIVATE.
# Finally, we supply a list of source files that will be built into the target. This is a standard
# CMake command.

file(GLOB_RECURSE NO_COMPILER_WARNINGS_DIR
		"modules/RnboExport/*h"
		)
set_source_files_properties(
		${NO_COMPILER_WARNINGS_DIR}
		PROPERTIES
		COMPILE_FLAGS "-w"
)



target_sources(${target_name}
		PRIVATE
		source/PluginEditor.cpp
		source/PluginEditor.h
		source/PluginProcessor.cpp
		source/PluginProcessor.h
		source/PluginParameters.cpp
		source/PluginParameters.h

		#utils
		source/utils/colors.h

		#look&feel
		source/ui/LookAndFeel/SliderLookAndFeel.cpp

		#XY Pad
		source/ui/CustomComponents/XYPad/XYPad.cpp
		source/ui/CustomComponents/XYPad/XYKnob.cpp
		source/ui/CustomComponents/XYPad/Arrow.cpp
		source/ui/CustomComponents/XYPad/ArrowButton.cpp

		#Transient Viewer
		source/ui/CustomComponents/TransientViewer/TransientViewer.cpp
		source/ui/CustomComponents/TransientViewer/TransientGrid.cpp

		#ui
		source/ui/CustomComponents/ParameterControl/ParameterControl.cpp
		source/ui/CustomComponents/ParameterControl/Slider/CustomSliderComponent.cpp
		source/ui/CustomComponents/ParameterControl/Slider/CustomComponents/CustomSlider.cpp
		source/ui/CustomComponents/ParameterControl/Slider/CustomComponents/CustomLabel.cpp

		#opengl
		source/ui/CustomComponents/OpenGLBackground/OpenGLBackground.cpp
		source/ui/CustomComponents/OpenGLBackground/OpenGLBackground.h
		source/ui/CustomComponents/OpenGLBackground/ShapeVertices.h
		source/ui/CustomComponents/OpenGLBackground/OpenGLUtil/OpenGLUtil.h

		#dry-wet-mixer
		source/dsp/mixer/DryWetMixer.cpp
		source/dsp/mixer/DryWetMixer.h

		#gain
		source/dsp/gain/ProcessorGain.cpp

		#grainDelay
		source/dsp/grainDelay/GrainDelay.cpp
		source/dsp/grainDelay/GrainDelay.h

		#analyser
		source/dsp/analyser/AudioVisualiser.cpp
		source/dsp/analyser/LevelAnalyser.cpp

		#AdvancedParameterControl
		source/ui/CustomComponents/AdvancedParameterControl/AdvancedParameterControl.h
		source/ui/CustomComponents/AdvancedParameterControl/AdvancedParameterControl.cpp

		#Texture
		source/ui/CustomComponents/Texture/TextureComponent.cpp

		source/ui/CustomComponents/Header/HeaderComponent.cpp

		#onnx
		source/dsp/onnx/InferenceThread.cpp
		source/dsp/onnx/InferenceThread.h
		source/dsp/onnx/OnnxProcessor.cpp
		source/dsp/onnx/OnnxProcessor.h
		source/dsp/onnx/RingBuffer.cpp
		source/dsp/onnx/RingBuffer.h

		#Filter
		source/dsp/Filter/IIRCutoffFilter.h
		source/dsp/Filter/IIRCutoffFilter.cpp

		#compressor
		source/dsp/compressor/Compressor.cpp
		source/dsp/compressor/Compressor.h
		source/dsp/compressor/ProcessorCompressor.cpp
		source/dsp/compressor/ProcessorCompressor.h

		#transientSplitter
		source/dsp/transientSplitter/TransientSplitter.cpp
		source/dsp/transientSplitter/TransientSplitter.h
		source/dsp/transientSplitter/ProcessorTransientSplitter.cpp
		source/dsp/transientSplitter/ProcessorTransientSplitter.h

		#utils
		source/dsp/utils/utils.cpp
		source/dsp/utils/utils.h
		source/dsp/utils/Envelope.cpp
		source/dsp/utils/Envelope.h

		#RNBO
		modules/RnboExport/rnbo/RNBO.h
		modules/RnboExport/rnbo/RNBO.cpp
		modules/RnboExport/rnbo_granular.cpp

		#other
		README.md
		)

# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.

target_compile_definitions(${target_name}
		PUBLIC
		# JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
		JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
		JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
		JUCE_VST3_CAN_REPLACE_VST2=0
		JUCE_DISPLAY_SPLASH_SCREEN=0
		DONT_SET_USING_JUCE_NAMESPACE=1
		)

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

# Automatically bundle libtorch into the targets

target_link_options(${target_name} PRIVATE -static)

set_property(TARGET ${target_name} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${target_name} PROPERTY CXX_STANDARD_REQUIRED ON)

# used to hotfix linking problem on unix
if (UNIX)
	set_property(TARGET BinaryData PROPERTY POSITION_INDEPENDENT_CODE ON)
endif (UNIX)

target_link_libraries(${target_name}
		PRIVATE
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
