set(BUILD_INFO_HEADER "${CMAKE_BINARY_DIR}/generated/BuildInfo.h")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")

add_custom_command(
    OUTPUT "${BUILD_INFO_HEADER}"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DOUTPUT_FILE=${BUILD_INFO_HEADER}
        -P ${CMAKE_SOURCE_DIR}/cmake/write_build_info.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating build info"
)

add_custom_target(GenerateBuildInfo DEPENDS "${BUILD_INFO_HEADER}")
add_dependencies(${TARGET_NAME} GenerateBuildInfo)

target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_BINARY_DIR}/generated")
