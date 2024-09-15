# Add the onnxruntime library
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
elseif (MSVC)
    include(cmake/download_onnx_win_debug.cmake)

    set_property(TARGET onnxruntime APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties(onnxruntime PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
            IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime/lib/onnxruntime-win-x64.lib"
    )
    set_target_properties(onnxruntime PROPERTIES
            MAP_IMPORTED_CONFIG_DEBUG Release
            MAP_IMPORTED_CONFIG_MINSIZEREL Release
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    )

    set_property(TARGET onnxruntime APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    set_target_properties(onnxruntime PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime-1.14.1-win-x86_64_Debug/onnxruntime-1.14.1-win-x86_64_Debug.lib"
    )
    set_target_properties(onnxruntime PROPERTIES
            MAP_IMPORTED_CONFIG_DEBUG Debug
    )
elseif(LINUX)
    set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime/lib/onnxruntime-linux_x86_64-static-combined.a)
endif()


# Link the onnxruntime library to the target
target_link_libraries(${TARGET_NAME} PRIVATE onnxruntime)
