# Static ONNX Runtime download via anira-project/backends (ORT 1.26.0). setup_anira.cmake hands
# SCYCLONE_ONNXRUNTIME_PACKAGE_DIR to anira as ANIRA_ONNXRUNTIME_ROOTDIR; anira builds its own
# anira::onnxruntime target from it, which is what Scyclone links. The IMPORTED onnxruntime target
# below is not linked by anything.

if(TARGET onnxruntime)
    return()
endif()

include(cmake/download_onnx_runtime.cmake)

add_library(onnxruntime STATIC IMPORTED)
scyclone_get_onnx_runtime_assets(_onnx_release_asset _onnx_debug_asset)

if(WIN32 AND CMAKE_CONFIGURATION_TYPES)
    scyclone_download_onnx_runtime_asset("${_onnx_release_asset}" _onnx_release_include _onnx_release_library)
    scyclone_download_onnx_runtime_asset("${_onnx_debug_asset}" _onnx_debug_include _onnx_debug_library)

    set(SCYCLONE_ONNXRUNTIME_INCLUDE_DIR "${_onnx_release_include}")
    # Package root = parent of include/ (Anira expects include/ + lib/ under ONNXRUNTIME_ROOTDIR).
    get_filename_component(SCYCLONE_ONNXRUNTIME_PACKAGE_DIR "${_onnx_release_include}" DIRECTORY)

    set_property(TARGET onnxruntime APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE DEBUG)
    set_target_properties(onnxruntime PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
        IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
        IMPORTED_LOCATION_RELEASE "${_onnx_release_library}"
        IMPORTED_LOCATION_DEBUG "${_onnx_debug_library}"
        MAP_IMPORTED_CONFIG_MINSIZEREL Release
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    )
else()
    if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(_onnx_asset "${_onnx_debug_asset}")
    else()
        set(_onnx_asset "${_onnx_release_asset}")
    endif()

    scyclone_download_onnx_runtime_asset("${_onnx_asset}" SCYCLONE_ONNXRUNTIME_INCLUDE_DIR _onnx_library)
    get_filename_component(SCYCLONE_ONNXRUNTIME_PACKAGE_DIR "${SCYCLONE_ONNXRUNTIME_INCLUDE_DIR}" DIRECTORY)

    set_target_properties(onnxruntime PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        IMPORTED_LOCATION "${_onnx_library}"
    )
endif()

set_target_properties(onnxruntime PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${SCYCLONE_ONNXRUNTIME_INCLUDE_DIR}"
)
