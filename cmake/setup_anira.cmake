# Anira inference integration — gated off for sanitizer stub builds.

if(SCYCLONE_SANITIZER_STUB_ONNX)
    message(STATUS "Sanitizer stub: Anira/ORT disabled (SCYCLONE_INFERENCE_STUB)")
    target_compile_definitions(${TARGET_NAME} PRIVATE SCYCLONE_INFERENCE_STUB=1)
    return()
endif()

include(cmake/setup_onnx_static_ort.cmake)

# Bring-your-own backend: hand Anira the ORT package Scyclone already downloaded (a tree with
# include/ + lib/) so it skips its own fetch. Anira derives engine linkage from BUILD_SHARED_LIBS
# alone (the per-engine ANIRA_<ID>_LINKAGE override was removed in v2.3.0), so the
# BUILD_SHARED_LIBS OFF wrapper around add_subdirectory below is what keeps ORT static.
set(ANIRA_ONNXRUNTIME_ROOTDIR "${SCYCLONE_ONNXRUNTIME_PACKAGE_DIR}" CACHE PATH "" FORCE)
unset(ANIRA_ONNXRUNTIME_LINKAGE CACHE)

# ONNX Runtime is the only backend Scyclone uses. LiteRT and ExecuTorch default to ON upstream
# and would pull down further prebuilt archives at configure time.
set(ANIRA_WITH_ONNXRUNTIME ON CACHE BOOL "" FORCE)
set(ANIRA_WITH_LIBTORCH OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_TFLITE OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_LITERT OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_EXECUTORCH OFF CACHE BOOL "" FORCE)

set(ANIRA_WITH_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_TESTS OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_BENCHMARK OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_DOCS OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_INSTALL OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_LOGGING OFF CACHE BOOL "" FORCE)

# Sanitizer flags come from Scyclone's own presets (ScycloneSanitizers.cmake). Anira's switches
# would add -fsanitize PUBLIC behind them, so keep them off.
set(ANIRA_WITH_RTSAN OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_ASAN OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_UBSAN OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_TSAN OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_LSAN OFF CACHE BOOL "" FORCE)

set(_scyclone_bsl ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# anira's msvc-support.cmake stops with "You need to specify CMAKE_BUILD_TYPE" whenever it is
# empty, which is always the case for Visual Studio and Ninja Multi-Config. Nothing else in anira
# reads it on this path (only its download and shared-library branches), so a placeholder that
# lives just for the add_subdirectory call is enough.
set(_scyclone_build_type_placeholder OFF)
if(MSVC AND SCYCLONE_GENERATOR_IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    set(_scyclone_build_type_placeholder ON)
endif()

add_subdirectory(modules/anira EXCLUDE_FROM_ALL)

if(_scyclone_build_type_placeholder)
    unset(CMAKE_BUILD_TYPE)
endif()
set(BUILD_SHARED_LIBS ${_scyclone_bsl} CACHE BOOL "" FORCE)

# anira::onnxruntime holds a single archive path taken from ANIRA_ONNXRUNTIME_ROOTDIR (the release
# package). With a multi-config generator the Debug configuration would link that release archive
# against the /MDd runtime (LNK2038), so swap it for a per-configuration pair. Whole list elements
# are matched after path normalisation, and anything unexpected stops the configure rather than
# silently leaving Debug broken.
if(WIN32 AND SCYCLONE_GENERATOR_IS_MULTI_CONFIG)
    get_target_property(_scyclone_ort_items anira::onnxruntime INTERFACE_LINK_LIBRARIES)
    file(TO_CMAKE_PATH "${SCYCLONE_ONNXRUNTIME_LIBRARY_RELEASE}" _scyclone_ort_release)
    set(_scyclone_ort_new_items "")
    set(_scyclone_ort_matches 0)
    foreach(_scyclone_ort_item IN LISTS _scyclone_ort_items)
        file(TO_CMAKE_PATH "${_scyclone_ort_item}" _scyclone_ort_item_normalised)
        if(_scyclone_ort_item_normalised STREQUAL _scyclone_ort_release)
            list(APPEND _scyclone_ort_new_items
                "$<$<CONFIG:Debug>:${SCYCLONE_ONNXRUNTIME_LIBRARY_DEBUG}>"
                "$<$<NOT:$<CONFIG:Debug>>:${SCYCLONE_ONNXRUNTIME_LIBRARY_RELEASE}>")
            math(EXPR _scyclone_ort_matches "${_scyclone_ort_matches} + 1")
        else()
            list(APPEND _scyclone_ort_new_items "${_scyclone_ort_item}")
        endif()
    endforeach()
    if(NOT _scyclone_ort_matches EQUAL 1)
        message(FATAL_ERROR
            "anira::onnxruntime no longer links exactly one release ORT archive "
            "(INTERFACE_LINK_LIBRARIES: '${_scyclone_ort_items}'); update the per-configuration "
            "ORT selection in cmake/setup_anira.cmake.")
    endif()
    set_property(TARGET anira::onnxruntime PROPERTY INTERFACE_LINK_LIBRARIES "${_scyclone_ort_new_items}")
endif()

# anira::anira is PUBLIC, not PRIVATE: the Test target links ${TARGET_NAME} and compiles sources
# that include <anira/anira.h>, so it needs the transitive link and include interface. Anira
# defines ANIRA_STATIC PUBLIC for static builds, which suppresses dllimport in anira/system/Exports.h.
#
# anira::onnxruntime: anira links ORT PRIVATE, but ScycloneModelConfig.cpp calls the Ort:: API
# directly (validateRaveModelFile). This is the consumer target anira documents for that — it
# carries the ORT headers and links the one archive anira already uses with its symbols hidden,
# so Scyclone never pulls in a second, exported copy of ORT.
target_link_libraries(${TARGET_NAME}
    PUBLIC anira::anira
    PRIVATE anira::onnxruntime)
