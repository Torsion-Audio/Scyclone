# Platform-specific settings
if(APPLE)
    set(MIN_MACOS_VERSION "10.13")
    set(CMAKE_OSX_DEPLOYMENT_TARGET ${MIN_MACOS_VERSION} CACHE STRING "Minimum version of the target platform" FORCE)

    # Check if the current deployment target is at least the minimum required version
    if(CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS MIN_MACOS_VERSION)
        message(FATAL_ERROR "macOS deployment target is too old. Minimum required version is ${MIN_MACOS_VERSION}. Current target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()

    set(FORMATS_TO_BUILD AU VST3 Standalone)
else()
    set(FORMATS_TO_BUILD VST3 Standalone)
endif()

# MSVC runtime linking
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") #static linking runtime library in Windows (for onnxruntime)
endif()