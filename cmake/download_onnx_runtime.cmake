include_guard(GLOBAL)

set(SCYCLONE_ONNXRUNTIME_VERSION "1.26.0" CACHE STRING "ONNX Runtime version downloaded from anira-project/backends")
set(SCYCLONE_ONNXRUNTIME_BACKENDS_TAG "v2.1.1" CACHE STRING "anira-project/backends release tag for ONNX Runtime binaries")
set(SCYCLONE_ONNXRUNTIME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/modules/onnxruntime" CACHE PATH "Local ONNX Runtime package root")
set(SCYCLONE_ONNXRUNTIME_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/import/onnxruntime" CACHE PATH "ONNX Runtime download cache")

function(_scyclone_normalize_onnx_arch OUTPUT_VARIABLE INPUT_ARCH)
    if(INPUT_ARCH MATCHES "^(x86_64|amd64|AMD64|x64)$")
        set(_normalized_arch "x86_64")
    elseif(INPUT_ARCH MATCHES "^(arm64|aarch64|ARM64)$")
        set(_normalized_arch "arm64")
    else()
        message(FATAL_ERROR "Unsupported ONNX Runtime architecture: ${INPUT_ARCH}")
    endif()

    set(${OUTPUT_VARIABLE} "${_normalized_arch}" PARENT_SCOPE)
endfunction()

function(_scyclone_macos_onnx_arch OUTPUT_VARIABLE)
    if(CMAKE_OSX_ARCHITECTURES)
        set(_architectures ${CMAKE_OSX_ARCHITECTURES})
        list(REMOVE_DUPLICATES _architectures)
        list(LENGTH _architectures _architecture_count)

        if(_architecture_count GREATER 1)
            list(FIND _architectures "arm64" _has_arm64)
            list(FIND _architectures "x86_64" _has_x86_64)
            if(_has_arm64 GREATER -1 AND _has_x86_64 GREATER -1)
                set(${OUTPUT_VARIABLE} "universal" PARENT_SCOPE)
                return()
            endif()
        endif()

        list(GET _architectures 0 _architecture)
    elseif(CMAKE_SYSTEM_PROCESSOR)
        set(_architecture "${CMAKE_SYSTEM_PROCESSOR}")
    else()
        set(_architecture "${CMAKE_HOST_SYSTEM_PROCESSOR}")
    endif()

    _scyclone_normalize_onnx_arch(_normalized_arch "${_architecture}")
    set(${OUTPUT_VARIABLE} "${_normalized_arch}" PARENT_SCOPE)
endfunction()

function(scyclone_get_onnx_runtime_assets OUTPUT_RELEASE_ASSET OUTPUT_DEBUG_ASSET)
    if(APPLE)
        _scyclone_macos_onnx_arch(_onnx_arch)
        set(_release_asset "onnxruntime-${SCYCLONE_ONNXRUNTIME_VERSION}-macOS-${_onnx_arch}-static.zip")
        set(_debug_asset "")
    elseif(WIN32)
        _scyclone_normalize_onnx_arch(_onnx_arch "${CMAKE_SYSTEM_PROCESSOR}")
        set(_release_asset "onnxruntime-${SCYCLONE_ONNXRUNTIME_VERSION}-Windows-${_onnx_arch}-static.zip")
        set(_debug_asset "onnxruntime-${SCYCLONE_ONNXRUNTIME_VERSION}-Windows-${_onnx_arch}-static-debug.zip")
    elseif(LINUX)
        _scyclone_normalize_onnx_arch(_onnx_arch "${CMAKE_SYSTEM_PROCESSOR}")
        if(_onnx_arch STREQUAL "arm64")
            set(_onnx_arch "aarch64")
        endif()
        set(_release_asset "onnxruntime-${SCYCLONE_ONNXRUNTIME_VERSION}-Linux-${_onnx_arch}-static.zip")
        set(_debug_asset "")
    else()
        message(FATAL_ERROR "No ONNX Runtime static build is configured for this platform")
    endif()

    set(${OUTPUT_RELEASE_ASSET} "${_release_asset}" PARENT_SCOPE)
    set(${OUTPUT_DEBUG_ASSET} "${_debug_asset}" PARENT_SCOPE)
endfunction()

function(scyclone_download_onnx_runtime_asset ASSET_NAME OUTPUT_INCLUDE_DIR OUTPUT_LIBRARY)
    string(REGEX REPLACE "\\.zip$" "" _asset_stem "${ASSET_NAME}")
    set(_package_dir "${SCYCLONE_ONNXRUNTIME_ROOT}/${_asset_stem}")
    set(_archive_path "${SCYCLONE_ONNXRUNTIME_DOWNLOAD_DIR}/${ASSET_NAME}")
    set(_stamp_path "${_package_dir}/.scyclone_complete")
    set(_url "https://github.com/anira-project/backends/releases/download/${SCYCLONE_ONNXRUNTIME_BACKENDS_TAG}/${ASSET_NAME}")

    if(NOT EXISTS "${_stamp_path}")
        message(STATUS "ONNX Runtime static package not found - downloading ${ASSET_NAME}")
        file(REMOVE_RECURSE "${_package_dir}")
        file(MAKE_DIRECTORY "${_package_dir}" "${SCYCLONE_ONNXRUNTIME_DOWNLOAD_DIR}")

        if(NOT EXISTS "${_archive_path}")
            file(DOWNLOAD
                "${_url}"
                "${_archive_path}"
                STATUS _download_status
                TLS_VERIFY ON
                SHOW_PROGRESS
            )
            list(GET _download_status 0 _download_status_code)
            list(GET _download_status 1 _download_status_message)

            if(NOT _download_status_code EQUAL 0)
                file(REMOVE "${_archive_path}")
                file(REMOVE_RECURSE "${_package_dir}")
                message(FATAL_ERROR "Failed to download ${ASSET_NAME}: ${_download_status_message}")
            endif()
        endif()

        file(ARCHIVE_EXTRACT INPUT "${_archive_path}" DESTINATION "${_package_dir}")
        file(WRITE "${_stamp_path}" "${SCYCLONE_ONNXRUNTIME_BACKENDS_TAG}/${ASSET_NAME}\n")
    endif()

    set(_include_dir "${_package_dir}/include")
    set(_library_candidates
        "${_package_dir}/lib/libonnxruntime.a"
        "${_package_dir}/lib/onnxruntime.lib"
    )

    foreach(_library_candidate IN LISTS _library_candidates)
        if(EXISTS "${_library_candidate}")
            set(_library "${_library_candidate}")
            break()
        endif()
    endforeach()

    if(NOT EXISTS "${_include_dir}")
        message(FATAL_ERROR "ONNX Runtime include directory not found in ${ASSET_NAME}")
    endif()

    if(NOT _library)
        message(FATAL_ERROR "ONNX Runtime static library not found in ${ASSET_NAME}")
    endif()

    set(${OUTPUT_INCLUDE_DIR} "${_include_dir}" PARENT_SCOPE)
    set(${OUTPUT_LIBRARY} "${_library}" PARENT_SCOPE)
endfunction()