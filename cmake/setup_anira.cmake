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
add_subdirectory(modules/anira EXCLUDE_FROM_ALL)
set(BUILD_SHARED_LIBS ${_scyclone_bsl} CACHE BOOL "" FORCE)

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
