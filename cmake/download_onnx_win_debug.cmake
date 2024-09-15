# Downloads and extracts pre-build ONNX-Runtime Debug lib for Windows

set(ONNX_RUNTIME_VERSION 1.14.1)
set(ONNX_RUNTIME_DEBUG_BASENAME onnxruntime-${ONNX_RUNTIME_VERSION}-win-x86_64_Debug)

# Define the directory path for the ONNX Runtime Debug library
set(ONNX_RUNTIME_DEBUG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules/${ONNX_RUNTIME_DEBUG_BASENAME})
set(ONNX_RUNTIME_ZIP ${ONNX_RUNTIME_DEBUG_BASENAME}.zip)

if(EXISTS ${ONNX_RUNTIME_DEBUG_DIR})
    message(STATUS "ONNX-Runtime Debug library found at ${ONNX_RUNTIME_DEBUG_DIR}")
else()
    message(STATUS "ONNX-Runtime Debug library not found - downloading pre-built library.")
    file(MAKE_DIRECTORY ${ONNX_RUNTIME_DEBUG_DIR})

    set(ONNX_RUNTIME_LIB_TYPE "zip")

    # Construct the URL for downloading the pre-built library
    set(ONNX_RUNTIME_URL https://github.com/timbencker/OnnxRuntimeBuilder/releases/download/${ONNX_RUNTIME_DEBUG_BASENAME}/${ONNX_RUNTIME_ZIP})
    message("Downloading from: " ${ONNX_RUNTIME_URL})

    # Define the path for the downloaded library file
    set(ONNX_RUNTIME_DOWNLOAD_PATH ${CMAKE_BINARY_DIR}/import/${ONNX_RUNTIME_ZIP})

    # Download the pre-built library
    file(DOWNLOAD ${ONNX_RUNTIME_URL} ${ONNX_RUNTIME_DOWNLOAD_PATH} STATUS ONNX_RUNTIME_DOWNLOAD_STATUS SHOW_PROGRESS)
    list(GET ONNX_RUNTIME_DOWNLOAD_STATUS 0 ONNX_RUNTIME_DOWNLOAD_STATUS_NO)

    file(ARCHIVE_EXTRACT
            INPUT ${ONNX_RUNTIME_DOWNLOAD_PATH}
            DESTINATION ${ONNX_RUNTIME_DEBUG_DIR})

    if(ONNX_RUNTIME_DOWNLOAD_STATUS_NO)
        message(STATUS "Pre-built library not downloaded. Error occurred, try again and check cmake/lib_onnxruntime.cmake")
        file(REMOVE_RECURSE ${ONNX_RUNTIME_DEBUG_DIR})
        file(REMOVE ${ONNX_RUNTIME_DOWNLOAD_PATH})
    else()
        message(STATUS "Linking downloaded ONNX-Runtime Debug pre-built library.")
    endif()
endif()