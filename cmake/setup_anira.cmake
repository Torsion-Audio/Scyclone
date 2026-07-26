# Anira inference integration — gated off for sanitizer stub builds.

if(SCYCLONE_SANITIZER_STUB_ONNX)
    message(STATUS "Sanitizer stub: Anira/ORT disabled (SCYCLONE_INFERENCE_STUB)")
    target_compile_definitions(${TARGET_NAME} PRIVATE SCYCLONE_INFERENCE_STUB=1)
    return()
endif()

include(cmake/setup_onnx_static_ort.cmake)

# Bring-your-own backend: hand Anira the ORT package Scyclone already downloaded (a tree with
# include/ + lib/) so it skips its own fetch. Anira resolves linkage from ANIRA_<ID>_LINKAGE,
# falling back to BUILD_SHARED_LIBS; pin it explicitly since Scyclone needs the static ORT.
set(ANIRA_ONNXRUNTIME_ROOTDIR "${SCYCLONE_ONNXRUNTIME_PACKAGE_DIR}" CACHE PATH "" FORCE)
set(ANIRA_ONNXRUNTIME_LINKAGE "static" CACHE STRING "" FORCE)

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

set(_scyclone_bsl ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(modules/anira EXCLUDE_FROM_ALL)
set(BUILD_SHARED_LIBS ${_scyclone_bsl} CACHE BOOL "" FORCE)

# PUBLIC, not PRIVATE: the Test target links ${TARGET_NAME} and compiles sources that include
# <anira/anira.h>, so it needs the transitive link and include interface.
#
# No ANIRA_EXPORTS needed: anira defines ANIRA_STATIC_DEFINE PUBLIC for static builds
# (cmake/msvc-support.cmake), which suppresses the dllimport decoration in AniraWinExports.h.
target_link_libraries(${TARGET_NAME} PUBLIC anira::anira)

if(WIN32)
    set_target_properties(anira PROPERTIES POSITION_INDEPENDENT_CODE OFF)
endif()
