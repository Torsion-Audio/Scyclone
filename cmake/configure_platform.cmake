# Platform-specific settings
if(APPLE)
    set(MIN_MACOS_VERSION "10.13")
    set(CMAKE_OSX_DEPLOYMENT_TARGET ${MIN_MACOS_VERSION} CACHE STRING "Minimum version of the target platform" FORCE)

    # Check if the current deployment target is at least the minimum required version
    if(CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS MIN_MACOS_VERSION)
        message(FATAL_ERROR "macOS deployment target is too old. Minimum required version is ${MIN_MACOS_VERSION}. Current target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()

    set(FORMATS_TO_BUILD AU VST3 Standalone)

    # Open-source Clang (Homebrew llvm, e.g. sanitizer CI jobs) compiles against its
    # bundled libc++ headers, whose ABI symbols (__cxa_init_primary_exception etc.)
    # the SDK's system libc++ tbd does not export. Link LLVM's own libc++ instead.
    # The LDFLAGS env propagates the same fix into JUCE's nested juceaide configure.
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        get_filename_component(_scyclone_llvm_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
        get_filename_component(_scyclone_llvm_root "${_scyclone_llvm_bin}" DIRECTORY)
        if(EXISTS "${_scyclone_llvm_root}/lib/c++")
            add_link_options("-L${_scyclone_llvm_root}/lib/c++" "-Wl,-rpath,${_scyclone_llvm_root}/lib/c++")
            set(ENV{LDFLAGS} "-L${_scyclone_llvm_root}/lib/c++ -Wl,-rpath,${_scyclone_llvm_root}/lib/c++ $ENV{LDFLAGS}")
        endif()
    endif()
else()
    set(FORMATS_TO_BUILD VST3 Standalone)
endif()

# MSVC runtime linking
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL") # match ONNX Runtime binaries built with the DLL CRT
endif()