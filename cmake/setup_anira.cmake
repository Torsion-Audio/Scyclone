# Anira inference integration — gated off for sanitizer stub builds.

if(SCYCLONE_SANITIZER_STUB_ONNX)
    message(STATUS "Sanitizer stub: Anira/ORT disabled (SCYCLONE_INFERENCE_STUB)")
    target_compile_definitions(${TARGET_NAME} PRIVATE SCYCLONE_INFERENCE_STUB=1)
    return()
endif()

include(cmake/setup_onnx_static_ort.cmake)

# Point Anira at Scyclone's downloaded ORT package — skip Anira's own download.
set(ONNXRUNTIME_ROOTDIR "${SCYCLONE_ONNXRUNTIME_PACKAGE_DIR}" CACHE PATH "" FORCE)

set(ANIRA_WITH_LIBTORCH OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_TFLITE OFF CACHE BOOL "" FORCE)
set(ANIRA_WITH_ONNXRUNTIME ON CACHE BOOL "" FORCE)
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
target_link_libraries(${TARGET_NAME} PUBLIC anira::anira)

if(WIN32)
    # anira/system/AniraWinExports.h has no static-library branch: without ANIRA_EXPORTS it
    # declares ANIRA_API as __declspec(dllimport), which is wrong for the static anira we build
    # above (LNK4217 / inconsistent dll linkage). Defining it here makes the consumer agree with
    # anira's own translation units. Side effect: anira's symbols are re-exported from the VST3.
    # Remove once upstream gains an ANIRA_STATIC guard.
    target_compile_definitions(${TARGET_NAME} PRIVATE ANIRA_EXPORTS)
    set_target_properties(anira PROPERTIES POSITION_INDEPENDENT_CODE OFF)
endif()
