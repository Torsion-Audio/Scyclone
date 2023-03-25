set(LIBONNXRUNTIME_VERSION 1.12.1)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime-${LIBONNXRUNTIME_VERSION}/)
    message(STATUS "ONNX-Runtime library found at /modules/onnxruntime-${LIBONNXRUNTIME_VERSION}")
else()
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime-${LIBONNXRUNTIME_VERSION}/)
    message(STATUS "ONNX-Runtime library not found - downloading pre-built library.")

    if(WIN32)
        set(LIB_ONNXRUNTIME_PRE_BUILD_LIB_NAME "onnxruntime-win-x64-${LIBONNXRUNTIME_VERSION}")
        set(LIB_ONNXRUNTIME_PRE_BUILD_LIB_TYPE "zip")
    endif()

    if(APPLE)
        if(NOT ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "arm64"))
            set(LIB_ONNXRUNTIME_PRE_BUILD_LIB_NAME "onnxruntime-osx-x86_64-${LIBONNXRUNTIME_VERSION}")
            set(LIB_ONNXRUNTIME_PRE_BUILD_LIB_TYPE "tgz")
        else()
            set(LIB_ONNXRUNTIME_PRE_BUILD_LIB_NAME "onnxruntime-osx-arm64-${LIBONNXRUNTIME_VERSION}")
            set(LIB_ONNXRUNTIME_PRE_BUILD_LIB_TYPE "tgz")
        endif()
    endif()

    if(UNIX AND NOT APPLE)
        set(LIB_ONNXRUNTIME_PRE_BUILD_LIB_NAME "onnxruntime-linux-x64-${LIBONNXRUNTIME_VERSION}")
        set(LIB_ONNXRUNTIME_PRE_BUILD_LIB_TYPE "tgz")
    endif()

    set(LIBONNXRUNTIME_URL https://github.com/microsoft/onnxruntime/releases/download/v${LIBONNXRUNTIME_VERSION}/${LIB_ONNXRUNTIME_PRE_BUILD_LIB_NAME}.${LIB_ONNXRUNTIME_PRE_BUILD_LIB_TYPE})
    set(LIBONNXRUNTIME_PATH ${CMAKE_BINARY_DIR}/import/${LIB_ONNXRUNTIME_PRE_BUILD_LIB_NAME}.${LIB_ONNXRUNTIME_PRE_BUILD_LIB_TYPE})

    file(DOWNLOAD ${LIBONNXRUNTIME_URL} ${LIBONNXRUNTIME_PATH} STATUS LIBONNXRUNTIME_DOWNLOAD_STATUS SHOW_PROGRESS)
    list(GET LIBONNXRUNTIME_DOWNLOAD_STATUS 0 LIBONNXRUNTIME_DOWNLOAD_STATUS_NO)

    file(ARCHIVE_EXTRACT
            INPUT ${CMAKE_BINARY_DIR}/import/${LIB_ONNXRUNTIME_PRE_BUILD_LIB_NAME}.${LIB_ONNXRUNTIME_PRE_BUILD_LIB_TYPE}
            DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime-${LIBONNXRUNTIME_VERSION}/)

    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime-${LIBONNXRUNTIME_VERSION}/${LIB_ONNXRUNTIME_PRE_BUILD_LIB_NAME}/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime-${LIBONNXRUNTIME_VERSION}/)

    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime-${LIBONNXRUNTIME_VERSION}/${LIB_ONNXRUNTIME_PRE_BUILD_LIB_NAME})

    if(LIBONNXRUNTIME_DOWNLOAD_STATUS_NO)
        message(STATUS "Pre-built library not downloaded. Error occurred, try again and check cmake/lib_onnxruntime.cmake")
        file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime-${LIBONNXRUNTIME_VERSION})
        file(REMOVE ${LIBONNXRUNTIME_PATH})
    else()
        message(STATUS "Linking downloaded ONNX-Runtime pre-built library.")
    endif()
endif()