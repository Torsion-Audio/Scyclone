# Windows MSVC toolchain for Ninja builds.
# Requires cmake/windows/generated/msvc-env.cmake from configure.ps1 (vcvars snapshot).

cmake_minimum_required(VERSION 3.21)

if(NOT WIN32)
    message(FATAL_ERROR "cmake/windows/msvc.toolchain.cmake is Windows-only.")
endif()

set(_scyclone_msvc_env "${CMAKE_CURRENT_LIST_DIR}/generated/msvc-env.cmake")
if(NOT EXISTS "${_scyclone_msvc_env}")
    message(FATAL_ERROR
        "Missing ${_scyclone_msvc_env}.\n"
        "Run .\\cmake\\windows\\configure.ps1 once to install/pin MSVC and generate the environment snapshot.")
endif()

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
    message(STATUS "Scyclone MSVC toolset: ${SCYCLONE_MSVC_TOOLSET}")
endif()
