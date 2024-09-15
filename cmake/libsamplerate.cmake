cmake_minimum_required(VERSION 3.15)

set(LIBSAMPLERATE_INSTALL OFF)
set(LIBSAMPLERATE_EXAMPLES OFF)
# libsamplerate itself does not require any dependencies,
# but if you want to build examples and tests, you will need the libsndfile and FFTW libraries
# Find dependencies (e.g., libsndfile if used)
# find_package(SndFile REQUIRED)

# Define the library name (as defined in libsamplerate's CMakeLists.txt)
set(LIBSAMPLERATE_LIB "samplerate")

# Check for 32-bit build on Windows
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(WARNING "You are trying to compile the project for a 32-bit (Win32) system. This might not work as expected. Please check https://github.com/libsndfile/libsamplerate/blob/master/docs/win32.md")
endif()

# Download libsamplerate from a remote repository
include(FetchContent)
FetchContent_Declare(
        libsamplerate
        GIT_REPOSITORY https://github.com/libsndfile/libsamplerate.git
        GIT_TAG 0.2.2  # The version of libsamplerate you wish to use
)
FetchContent_MakeAvailable(libsamplerate)

if(NOT TARGET ${LIBSAMPLERATE_LIB})
    message(FATAL_ERROR "Failed to build or find libsamplerate")
endif()


# Used in main CMakeLists.txt to link the target
target_link_libraries(${TARGET_NAME} PRIVATE ${LIBSAMPLERATE_LIB})

