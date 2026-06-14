# Add the onnxruntime library
if(SCYCLONE_SANITIZER_STUB_ONNX)
  message(STATUS "Sanitizer stub: ONNX Runtime link disabled (SCYCLONE_ONNX_STUB)")
  target_compile_definitions(${TARGET_NAME} PRIVATE SCYCLONE_ONNX_STUB=1)
else()

add_library(onnxruntime STATIC IMPORTED)

if (APPLE)
    message(STATUS ${CMAKE_HOST_SYSTEM_PROCESSOR})
    if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime/lib/onnxruntime-osx-x64.a)
    elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
        set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime/lib/onnxruntime-osx-arm64.a)
        set_property(TARGET onnxruntime PROPERTY CXX_VISIBILITY_PRESET default)
    else()
        message(FATAL_ERROR "CMAKE_HOST_SYSTEM_PROCESSOR not defined.")
    endif()
elseif (WIN32)
    include(cmake/download_onnx_win_debug.cmake)

    set(_onnx_release "${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime/lib/onnxruntime-win-x64.lib")
    set(_onnx_debug "${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime-1.14.1-win-x86_64_Debug/onnxruntime-1.14.1-win-x86_64_Debug.lib")

    if (CMAKE_CONFIGURATION_TYPES)
        # Multi-config generators (Visual Studio, Xcode)
        set_property(TARGET onnxruntime APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE DEBUG)
        set_target_properties(onnxruntime PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
                IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
                IMPORTED_LOCATION_RELEASE "${_onnx_release}"
                IMPORTED_LOCATION_DEBUG "${_onnx_debug}"
                MAP_IMPORTED_CONFIG_MINSIZEREL Release
                MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
        )
    else()
        # Single-config generators (Ninja, NMake): plain IMPORTED_LOCATION only
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(_onnx_selected "${_onnx_debug}")
        else()
            set(_onnx_selected "${_onnx_release}")
        endif()
        set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION "${_onnx_selected}")
    endif()
elseif(LINUX)
    set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime/lib/onnxruntime-linux_x86_64-static-combined.a)
endif()


# Link the onnxruntime library to the target
target_link_libraries(${TARGET_NAME} PRIVATE onnxruntime)

endif()
