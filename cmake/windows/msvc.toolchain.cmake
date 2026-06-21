# Windows MSVC toolchain for Ninja builds.
#
# Local dev: cmake/windows/generated/msvc-env.cmake from ensure-msvc.ps1 bakes include/lib
# paths so cmake --build works without vcvars in every shell.
#
# CI: when LIB is already set (e.g. setup-msvc / GITHUB_ENV), skip the snapshot and let
# CMake use the active vcvars environment.

cmake_minimum_required(VERSION 3.21)

if(NOT WIN32)
    message(FATAL_ERROR "cmake/windows/msvc.toolchain.cmake is Windows-only.")
endif()

set(_scyclone_msvc_env "${CMAKE_CURRENT_LIST_DIR}/generated/msvc-env.cmake")

if(EXISTS "${_scyclone_msvc_env}")
    include("${_scyclone_msvc_env}")

    if(NOT SCYCLONE_MSVC_CL OR NOT SCYCLONE_MSVC_CXX)
        message(FATAL_ERROR "Invalid MSVC environment snapshot: compiler paths are missing.")
    endif()

    set(CMAKE_C_COMPILER "${SCYCLONE_MSVC_CL}" CACHE FILEPATH "C compiler" FORCE)
    set(CMAKE_CXX_COMPILER "${SCYCLONE_MSVC_CXX}" CACHE FILEPATH "CXX compiler" FORCE)

    foreach(_dir IN LISTS SCYCLONE_MSVC_LIB_DIRS)
        if(_dir)
            list(APPEND CMAKE_C_IMPLICIT_LINK_DIRECTORIES "${_dir}")
            list(APPEND CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "${_dir}")
            string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " /LIBPATH:\"${_dir}\"")
            string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT " /LIBPATH:\"${_dir}\"")
            string(APPEND CMAKE_MODULE_LINKER_FLAGS_INIT " /LIBPATH:\"${_dir}\"")
        endif()
    endforeach()

    foreach(_dir IN LISTS SCYCLONE_MSVC_INCLUDE_DIRS)
        if(_dir)
            list(APPEND CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES "${_dir}")
            list(APPEND CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "${_dir}")
            string(APPEND CMAKE_C_FLAGS_INIT " /I\"${_dir}\"")
            string(APPEND CMAKE_CXX_FLAGS_INIT " /I\"${_dir}\"")
        endif()
    endforeach()

    if(SCYCLONE_MSVC_TOOLSET)
        message(STATUS "Scyclone MSVC toolset (snapshot): ${SCYCLONE_MSVC_TOOLSET}")
    endif()
elseif(DEFINED ENV{LIB} AND NOT "$ENV{LIB}" STREQUAL "")
    message(STATUS "Scyclone MSVC toolset: using vcvars environment (LIB is set)")
elseif(DEFINED ENV{INCLUDE} AND NOT "$ENV{INCLUDE}" STREQUAL "")
    message(STATUS "Scyclone MSVC toolset: using vcvars environment (INCLUDE is set)")
else()
    message(FATAL_ERROR
        "Missing ${_scyclone_msvc_env} and no MSVC environment detected.\n"
        "Run .\\cmake\\windows\\ensure-msvc.ps1 once to install/pin MSVC and generate the environment snapshot.")
endif()
