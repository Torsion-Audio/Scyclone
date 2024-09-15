# Collect all source files (.cpp and .h) in the 'source' directory
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
)

# list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/ui/CustomComponents/OpenGLBackground/OpenGLUtil/OpenGLUtil.h)

# Add all sources to target
target_sources(${TARGET_NAME} PRIVATE ${SOURCES} ${RNBO_SOURCES})

# Add include directories for all folders in the source
file(GLOB_RECURSE SOURCE_DIRS LIST_DIRECTORIES true
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*
)
list(APPEND SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source)

# Add include directories for all directories found in 'source'
target_include_directories(${TARGET_NAME} PRIVATE
        ${SOURCE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime/include
)

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime/include)

# Make the folder structure visible in the IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX "source" FILES ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/modules PREFIX "modules" FILES ${RNBO_SOURCES})