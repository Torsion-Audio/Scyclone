# Collect all source files (.cpp and .h) in the 'source' directory
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
)

# Inference backend split: Anira (release) vs sanitizer delay-line stub.
if(SCYCLONE_SANITIZER_STUB_ONNX)
    list(FILTER SOURCES EXCLUDE REGEX ".*/AniraInferenceBackend\\.(cpp|h)$")
    list(FILTER SOURCES EXCLUDE REGEX ".*/ScycloneModelConfig\\.(cpp|h)$")
else()
    list(FILTER SOURCES EXCLUDE REGEX ".*/SanitizerInferenceBackend\\.(cpp|h)$")
endif()

# Add all sources to target
target_sources(${TARGET_NAME} PRIVATE ${SOURCES} ${RNBO_SOURCES})

# Add include directories for all folders in the source
file(GLOB_RECURSE SOURCE_DIRS LIST_DIRECTORIES true
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*
)
list(APPEND SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source)

# Prefer include dir from downloaded ORT package (set by setup_onnx_static_ort.cmake).
if(NOT SCYCLONE_ONNXRUNTIME_INCLUDE_DIR)
    set(SCYCLONE_ONNXRUNTIME_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime/include)
endif()

target_include_directories(${TARGET_NAME} PRIVATE
        ${SOURCE_DIRS}
        ${SCYCLONE_ONNXRUNTIME_INCLUDE_DIR}
)

# Make the folder structure visible in the IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX "source" FILES ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/modules PREFIX "modules" FILES ${RNBO_SOURCES})
